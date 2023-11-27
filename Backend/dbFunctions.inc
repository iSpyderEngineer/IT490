<?php
require_once('dbConnection.inc');
require_once('rabbitFunctions.inc');

//isfar
function validateLogin($username, $password) {
    $database = dbConnect();
    $sanitizedUsername = $database->real_escape_string($username);
    $sanitizedPassword = $database->real_escape_string($password);
    $query = "SELECT * FROM Accounts WHERE Username = '$sanitizedUsername'";
    $result = $database->query($query);

    while ($row = $result->fetch_assoc()) {
        if ($sanitizedPassword === $row["PasswordHash"]) {
                return json_encode(array("returnCode" => '1', 'message' => "Logged In"));
            } else {
                return json_encode(array("returnCode" => '0', 'message' => "Error with generating session"));
            }
        
    }
    return json_encode(array("returnCode" => '0', 'message' => "No User Match The Given Username"));
}

//isfar
function validateRegister($firstname, $lastname, $username, $email, $address, $city, $country, $zipcode, $password) {
    $database = dbConnect();
    $sanitizedFirstname = $database->real_escape_string($firstname);
    $sanitizedLastname = $database->real_escape_string($lastname);
    $sanitizedUsername = $database->real_escape_string($username);
    $sanitizedEmail = $database->real_escape_string($email);
    $sanitizedAddress = $database->real_escape_string($address);
    $sanitizedCity = $database->real_escape_string($city);
    $sanitizedCountry= $database->real_escape_string($country);
    $sanitizedZipcode = $database->real_escape_string($zipcode);
    $sanitizedPassword= $database->real_escape_string($password);
    $query = "SELECT * FROM Accounts WHERE Username = '$sanitizedUsername'";
    $result = $database->query($query);

    while ($row = $result->fetch_assoc()) {
        if ($row["Username"] == $sanitizedUsername) {
            return json_encode(array("returnCode" => '0', 'message' => "Username already exists, try again"));
        }
        if ($row["Email"] == $sanitizedEmail) {
            return json_encode(array("returnCode" => '0', 'message'=>"Email already exists, try again"));
        }
    }
    $insertQuery = "INSERT INTO Accounts (FirstName, LastName, Username, Email, Address, City, Country, ZipCode, PasswordHash) VALUES ('$sanitizedFirstname', '$sanitizedLastname', '$sanitizedUsername', '$sanitizedEmail', '$sanitizedAddress', '$sanitizedCity', '$sanitizedCountry', '$sanitizedZipcode', '$sanitizedPassword')";
    if ($database->query($insertQuery) === TRUE) {
        return json_encode(array("returnCode" => '1', 'message' => "Username and password accepted"));
    }
    return json_encode(array("returnCode" => '0', 'message' => "Error with registering, try again"));
}

//isfar
function validatePreferences($username, $favoriteActor, $favoriteDirector, $favoriteMovie, $favoriteGenre, $biography) {
    $accountID = getAccountIdByUsername($username);
    if (!$accountID) {
        return array('error' => 'Username not found.');
    }
    $mysqli = dbConnect();
 //update sql   $query = "UPDATE Users SET FavoriteActor = ?, FavoriteDirector = ?, FavoriteMovie = ?, FavoriteGenre = ?, Biography = ? WHERE UserID = ?";
    if ($stmt = $mysqli->prepare($query)) {
        $stmt->bind_param("sssssi", $favoriteActor, $favoriteDirector, $favoriteMovie, $favoriteGenre, $biography, $accountID);
        if ($stmt->execute()) {
            $stmt->close();
            $mysqli->close();
            return array('success' => 'User preferences updated successfully.');
        } else {
            $stmt->close();
            $mysqli->close();
            return array('error' => 'Error updating user preferences.');
        }
    }
    return array('error' => 'Error updating user preferences.');
}

//isfar
function getUserProfileData($username) {
    $mysqli = dbConnect();
    $accountID = getAccountIdByUsername($username);
    if (!$accountID) {
        return null;
    }
  //update sql statement to match  $query = "SELECT U.FavoriteDirector, U.FavoriteGenre, U.FavoriteMovie, U.FavoriteActor, U.Bio, R.MovieTitle, R.RateScore FROM Users U LEFT JOIN user_ratings R ON U.UserID = R.UserID WHERE U.UserID = ?";
    if ($stmt = $mysqli->prepare($query)) {
        $stmt->bind_param("i", $accountID);
        $stmt->execute();
        $stmt->bind_result($favoriteDirector, $favoriteGenre, $favoriteMovie, $favoriteActor, $bio, $rateScore);
        $userData = [
            "FavoriteDirector" => $favoriteDirector,
            "FavoriteGenre" => $favoriteGenre,
            "FavoriteMovie" => $favoriteMovie,
            "FavoriteActor" => $favoriteActor,
            "Bio" => $bio,
        ];
        while ($stmt->fetch()) {
            $userData['RatedMovies'][] = [
                'RateScore' => $rateScore,
            ];
        }
        $stmt->close();
        $mysqli->close();
        return $userData;
    }
    return null;
}

//isfar
/*
function updateProfileSettings($username, $favActor, $favGenre, $favDirector, $favMovie, $biography) {
    $mysqli = dbConnect(); 
    $AccountID = getAccountID($username);
    $sanitizedFavoriteActor = $database->real_escape_string($favActor);
    $sanitizedFavoriteGenre = $database->real_escape_string($favGenre);
    $sanitizedFavoriteDirector= $database->real_escape_string($favDirector);
    $sanitizedFavoriteMovie = $database->real_escape_string($favMovie);
    $sanitizedBiography= $database->real_escape_string($biography);
    $updateQuery = "UPDATE Profiles SET FavoriteDirector = $sanitizedFavoriteDirector, FavoriteGenre = $sanitizedFavoriteGenre, FavoriteMovie = $sanitizedFavoriteMovie, FavoriteActor = $sanitizedFavoriteActor, Bio = $sanitizedBiography WHERE AccountID = $AccountID";
    
    if ($updateStmt = $mysqli->prepare($updateQuery)) {
        $updateStmt->bind_param("ssssss", $data['FavoriteDirector'], $data['FavoriteGenre'], $data['FavoriteMovie'], $data['FavoriteActor'], $data['Bio'], $sessionID);
        
        if ($updateStmt->execute()) {
            $updateStmt->close();
            return ["status" => 200, "message" => "Settings updated successfully"];
        }
        
        $updateStmt->close();
    }

    return ["status" => 500, "message" => "Failed to update settings"]; 
}

 $mysqli = dbConnect();
    $mysqli->close(); 
}
*/

//isfar
function getWatchListData($username) {
    $accountId = getAccountIdByUsername($username);
    if (!$accountId) {
        return array('error' => 'Username not found.');
    }
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    $watchListData = array();
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    } else {
        $sql = "SELECT Movie, PosterURL, Year FROM WatchList WHERE AccountID = ?";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("i", $accountId);
        $stmt->execute();
        $result = $stmt->get_result();
        while ($row = $result->fetch_assoc()) {
            $watchListData[] = array(
                'MovieTitle' => $row['MovieTitle'],
                'PosterURL' => $row['PosterURL'],
                'Year' => $row['Year'],
            );
        }
        $stmt->close();
        $conn->close();
    }
    return $watchListData;
}

//isfar
function getWatchedListData($username) {
    $accountId = getAccountIdByUsername($username);
    if (!$accountId) {
        return array('error' => 'Username not found.');
    }
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    $watchedListData = array();
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    } else {
     $sql = "SELECT Movie, PosterURL, Year FROM WatchedList WHERE AccountID = ?";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("i", $accountId);
        $stmt->execute();
        $result = $stmt->get_result();
        while ($row = $result->fetch_assoc()) {
            $watchedListData[] = array(
                'MovieTitle' => $row['MovieTitle'],
                'PosterURL' => $row['PosterURL'],
                'Year' => $row['Year'],
            );
        }
        $stmt->close();
        $conn->close();
    }
    return $watchedListData;
}

//isfar
function addToWatchList($username, $movieTitle, $posterURL, $year) {
    $accountId = getAccountIdByUsername($username);
    if (!$accountId) {
        return array('success' => false, 'message' => 'Username not found.');
    }
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    $status = [];
    if ($conn->connect_error) {
        $status['success'] = false;
        $status['message'] = "Connection failed: " . $conn->connect_error;
    } else {
        $sql = "INSERT INTO WatchList (AccountID, Movie, PosterURL, Year, TimeCreated, LastModified) VALUES ('$accountId', '$movieTitle', '$posterURL', '$year', NOW(), NOW())";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("isss", $accountId, $movieTitle, $posterURL, $year);
        if ($stmt->execute()) {
            $status['success'] = true;
            $status['message'] = "Added successfully.";
        } else {
            $status['success'] = false;
            $status['message'] = "Error: " . $stmt->error;
        }
        $stmt->close();
    }
    $conn->close();
    sendStatusToRabbitMQ(json_encode($status));
    return $status;
}

//isfar
function addToWatchedList($username, $movieTitle, $posterURL, $year) {
    $accountId = getAccountIdByUsername($username);
    if (!$accountId) {
        return array('success' => false, 'message' => 'Username not found.');
    }
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    $status = [];
    if ($conn->connect_error) {
        $status['success'] = false;
        $status['message'] = "Connection failed: " . $conn->connect_error;
    } else {
        $sql = "INSERT INTO WatchedList (AccountID, Movie, PosterURL, Year, TimeCreated, LastModified) VALUES ('$accountId', '$movieTitle', '$posterURL', '$year', NOW(), NOW())";
        $stmt = $conn->prepare($sql);
       // $stmt->bind_param("isss", $accountId, $movieTitle, $posterURL, $year);
        if ($stmt->execute()) {
            $status['success'] = true;
            $status['message'] = "Added to watched list successfully.";
        } else {
            $status['success'] = false;
            $status['message'] = "Error: " . $stmt->error;
        }
        $stmt->close();
    }
    $conn->close();
    sendStatusToRabbitMQ(json_encode($status));
    return $status;
}

//isfar
function handlePostRequest() {
    $data = json_decode(file_get_contents('php://input'), true);
    if (!empty($data) && isset($data['session_id'])) {
        $sessionID = $data['session_id'];
        $result = updateProfileSettings($sessionID, $data);
        http_response_code($result['status']);
        echo json_encode(["message" => $result['message']]);
        exit;
    }
    echo "Invalid or missing data."; 
}

function handleInvalidRequestMethod() {
    echo "Invalid request method."; 
}

function processRequest() {
    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        handlePostRequest();
    } else {
        handleInvalidRequestMethod();
    }

    $mysqli = dbConnect();
    $mysqli->close(); 
}

//isfar
function updateUserProfile($username, $favoriteActor, $favoriteMovie, $favoriteDirector, $favoriteGenres, $biography) {
    $mysqli = dbConnect();
    $accountID = getAccountIdByUsername($username);
    if (!$accountID) {
        return false;
}
    $query = "UPDATE Profiles SET FavoriteActor = '$favoriteActor', FavoriteMovie = '$favoriteMovie', FavoriteDirector= '$favoriteDirector', FavoriteGenres = '$favoriteGenres', Biography = '$biography' WHERE AccountID = '$accountID'; ";
    if ($stmt = $mysqli->prepare($query)) {
       // $stmt->bind_param("sssssi", $favoriteActor, $favoriteMovie, $favoriteDirector, $favoriteGenres, $biography, $accountID);
        if ($stmt->execute()) {
            $stmt->close();
            $mysqli->close();
            return true;
        } else {
            $stmt->close();
            $mysqli->close();
            return false; 
        }
    }
    return false;
}

//isfar
function getAccountIdByUsername($username) {
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    $username = $conn->real_escape_string($username);
     $sql = "SELECT AccountID FROM Accounts WHERE Username = '$username'";
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $accountId = $row['id'];
        $conn->close();
        return $accountId;
    } else {
        $conn->close();
        return false; // Username not found
    }
}
    
function calculateRateScoreAndLeaderboard($connection) {
$connection = new mysqli('10.244.1.2', 'BackendAdmin', 'Qg5OKQ!?$Q', 'SceneSync');

if ($connection->connect_error) {
    die("Connection failed: " . $connection->connect_error);
    $query = "SELECT Profiles.RateScore, Profiles.AccountID, Accounts.Username 
              FROM Profiles 
              CROSS JOIN Accounts 
              WHERE Accounts.AccountID = Profiles.AccountID 
              ORDER BY RateScore DESC 
              LIMIT 0, 10";
    
    $result = mysqli_query($connection, $query);
    if (!$result) {
        die('Error in query: ' . mysqli_error($connection));
    }
    $leaderboard = [];
    while ($row = mysqli_fetch_assoc($result)) {
        $leaderboard[] = $row;
    }
    mysqli_free_result($result);
    return $leaderboard;
}
}
//isfar ==== unique identifier for movie?
/*
function deleteFromWatchList($watchListID) {
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    $status = [];
    if ($conn->connect_error) {
        $status['success'] = false;
        $status['message'] = "Connection failed: " . $conn->connect_error;
    } else {
        $sql = "DELETE FROM WatchList WHERE WatchListID = ?";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("i", $watchListID);
        if ($stmt->execute()) {
            $status['success'] = true;
            $status['message'] = "Deleted successfully from WatchList.";
        } else {
            $status['success'] = false;
            $status['message'] = "Error: " . $stmt->error;
        }
        $stmt->close();
        $conn->close();
    }
    return $status;
}
*/

/**
 * Inserts a user review into the database
 * @param int $accountId the account ID of the user who left the review
 * @param string $movieTitle the title of the movie
 * @param int $rating a rating (1-5) left by the user
 * @param string $review movie review left by the user
 */
function insertReview($accountId, $movieTitle, $rating, $review){
    // CREATE TABLE Rating(AccountID INT UNSIGNED NOT NULL, movieTitle VARCHAR(255) NOT NULL, Rating TINYINT UNSIGNED NOT NULL, Review TEXT NOT NULL);
    $connection = dbConnect();
    $stmt = $connection->prepare("INSERT INTO Rating(AccountID, movieTitle, Rating, Review) VALUES(?, ?, ?, ?)");
    $stmt->bind_param("dsds", $accountId, $movieTitle, $rating, $review);
    $stmt->execute();
    $succeeded = $stmt->errno === 0;
    $stmt->close();
    $connection->close();
    http_response_code($succeeded ? 200 : 500);
    return json_encode(["returnCode" => $succeeded ? 1 : 0]);
}

/**
 * Searches ratings for a given movie title and returns a JSON array.
 * @param string $movieTitle the movie title to search ratings for
 * @return string
 */
function searchRatingsByMovie($movieTitle){
    $result = [];
    $connection = dbConnect();
    $stmt = $connection->prepare("SELECT AccountID, movieTitle, Rating, Review FROM Rating WHERE movieTitle=?");
    $stmt->bind_param("s", $movieTitle);
    $stmt->execute();
    $stmt->bind_result($accountId, $movieTitle, $rating, $review);
    while($stmt->fetch()){
        $result[] = [
            "AccountID" => $accountId,
            "movieTitle" => $movieTitle,
            "Rating" => $rating,
            "Review" => $review
        ];
    }
    $stmt->close();
    $connection->close();
    http_response_code(200);
    return json_encode(["returnCode" => '1', "reviews" => $result]);
}






?>
