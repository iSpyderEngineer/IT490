<?php
require_once('dbConnection.inc');

error_reporting(E_ALL);
ini_set('display_errors', 'On');

//isfar + Sean
function validateLogin($username, $password) {
    echo "Starting validation" . PHP_EOL;
    $database = dbConnect();
    echo "Database connected" . PHP_EOL;
    $sanitizedUsername = $database->real_escape_string($username);
    $sanitizedPassword = $database->real_escape_string($password);
    $query = "SELECT * FROM Accounts WHERE Username = '$sanitizedUsername'";
    echo "Executing query: $query" . PHP_EOL;
    $result = $database->query($query);
    $usernameFound = false;
    $startTime = microtime(true);
    while ($row = $result->fetch_assoc()) {
        $usernameFound = true;
        if ($sanitizedPassword === $row["PasswordHash"]) {
            echo "Password match found" . PHP_EOL;
            return json_encode(array("returnCode" => '1', 'message' => "Logged In"));
        } else {
            // Check elapsed time
            $elapsedTime = microtime(true) - $startTime;

            // Set a time limit, e.g., 5 seconds
            $timeLimit = 5;
            if ($elapsedTime > $timeLimit) {
                echo "Password check took too long" . PHP_EOL;
                return json_encode(array("returnCode" => '0', 'message' => "Password check is taking too long"));
            }
            // Continue checking passwords
            echo "Continuing password check" . PHP_EOL;
        }
    }
    if ($usernameFound) {
        echo "Incorrect Password" . PHP_EOL;
        return json_encode(array("returnCode" => '0', 'message' => "Incorrect Password"));
    } else {
        echo "No User Match The Given Username" . PHP_EOL;
        return json_encode(array("returnCode" => '0', 'message' => "No User Match The Given Username"));
    }
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
            return array("returnCode" => '0', "message" => "Username already exists, try again");
        }
        if ($row["Email"] == $sanitizedEmail) {
            return array("returnCode" => '0', 'message'=>"Email already exists, try again");
        }
    }
    $insertQuery = "INSERT INTO Accounts (FirstName, LastName, Username, Email, Address, City, Country, ZipCode, PasswordHash) VALUES ('$sanitizedFirstname', '$sanitizedLastname', '$sanitizedUsername', '$sanitizedEmail', '$sanitizedAddress', '$sanitizedCity', '$sanitizedCountry', '$sanitizedZipcode', '$sanitizedPassword')";
    if ($database->query($insertQuery) === TRUE) {
        return (array("returnCode" => '1', 'message' => "Username and password accepted"));
    }
    return (array("returnCode" => '0', 'message' => "Error with registering, try again"));
}

//isfar
function validatePreferences($username, $favoriteActor, $favoriteDirector, $favoriteMovie, $favoriteGenre, $biography){
    $accountID = getAccountIdByUsername($username);
    if (!$accountID) {
        return array('error' => 'Username not found.');
    }
    $mysqli = dbConnect();
    $query = "INSERT INTO Profiles (AccountID, FavoriteActor, FavoriteDirector, FavoriteMovie, FavoriteGenres, Biography) VALUES (?, ?, ?, ?, ?, ?)";
    if ($stmt = $mysqli->prepare($query)) {
        $stmt->bind_param("isssss", $accountID, $favoriteActor, $favoriteDirector, $favoriteMovie, $favoriteGenre, $biography );
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
    $query = "SELECT FavoriteDirector, FavoriteGenres, FavoriteMovie, FavoriteActor, Biography, RateScore FROM Profiles WHERE AccountID = ?";
    if ($stmt = $mysqli->prepare($query)) {
        $stmt->bind_param("i", $accountID);
        $stmt->execute();
        $stmt->bind_result($favoriteDirector, $favoriteGenre, $favoriteMovie, $favoriteActor, $bio, $rateScore);
        while ($stmt->fetch()) {
            $userData[] = [
                "FavoriteDirector" => $favoriteDirector,
                "FavoriteGenres" => $favoriteGenre,
                "FavoriteMovie" => $favoriteMovie,
                "FavoriteActor" => $favoriteActor,
                 "Biography" => $bio,
                 "RateScore" => $rateScore
            ];
        }
        $stmt->close();
        $mysqli->close();
        return $userData;
    }
    return null;
}

//isfar

function updateProfileSettings($username, $favActor, $favGenre, $favDirector, $favMovie, $biography) {
    $mysqli = dbConnect();
    $AccountId = getAccountIdByUsername($username);
    $updateQuery = "UPDATE Profiles SET FavoriteDirector = ?, FavoriteGenre = ?, FavoriteMovie = ?, FavoriteActor = ?, Bio = ? WHERE AccountID = ?";
    if ($updateStmt = $mysqli->prepare($updateQuery)) {
        $updateStmt->bind_param("sssssi", $favDirector, $favGenre, $favMovie, $favActor, $biography, $AccountID);
        if ($updateStmt->execute()) {
            $updateStmt->close();
            $mysqli->close();
            return ["status" => 200, "message" => "Settings updated successfully"];
        }
        $updateStmt->close();
    }
    $mysqli->close();
    return ["status" => 500, "message" => "Failed to update settings"];
}

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
        $sql = "SELECT ListID, MovieID, Movie, PosterURL, Year, MediaType FROM WatchList WHERE AccountID = ?";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("i", $accountId);
        $stmt->execute();
        $result = $stmt->get_result();
        while ($row = $result->fetch_assoc()) {
            $watchListData[] = array(
                'MovieID' => $row['MovieID'],
                'MovieTitle' => $row['Movie'],
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
     $sql = "SELECT ListID, MovieID, Movie, PosterURL, Year, MediaType FROM WatchedList WHERE AccountID = ?";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("i", $accountId);
        $stmt->execute();
        $result = $stmt->get_result();
        while ($row = $result->fetch_assoc()) {
            $watchedListData[] = array(
                'MovieID' => $row['MovieID'],
                'MovieTitle' => $row['Movie'],
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
function addToWatchList($username, $MovieID, $movieTitle, $posterURL, $year, $mediatype) {
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
        $sql = "INSERT INTO WatchList (AccountID, MovieID, Movie, PosterURL, Year, MediaType, TimeCreated, LastModified) VALUES (?, ?, ?, ?, ?, ?, NOW(), NOW())";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("isssss", $accountId, $MovieID, $movieTitle, $posterURL, $year, $mediatype);
        if ($stmt->execute()) {
            $movieID = $stmt->insert_id; // Retrieve the last inserted ID
            $status['success'] = true;
            $status['message'] = "Added successfully. MovieID: $movieID";
        } else {
            $status['success'] = false;
            $status['message'] = "Error: " . $stmt->error;
        }
        $stmt->close();
    }
    $conn->close();
   // sendStatusToRabbitMQ(json_encode($status));
    return $status;
}

//isfar
function addToWatchedList($username, $MovieID, $movieTitle, $posterURL, $year, $mediatype) {
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
        $sql = "INSERT INTO WatchedList (AccountID, MovieID, Movie, PosterURL, Year, MediaType) VALUES (?, ?, ?, ?, ?, ?)";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("isssss", $accountId, $MovieID, $movieTitle, $posterURL, $year, $mediatype);
        if ($stmt->execute()) {
            $movieID = $stmt->insert_id; // Retrieve the last inserted ID
            $status['success'] = true;
            $status['message'] = "Added to watched list successfully. MovieID: $movieID";
        } else {
            $status['success'] = false;
            $status['message'] = "Error: " . $stmt->error;
        }
        $stmt->close();
    }
    $conn->close();
    // sendStatusToRabbitMQ(json_encode($status));
    return $status;
}

//isfar
function addToWatchedListAndRemoveFromWatchList($username, $MovieID, $movieTitle, $posterURL, $year, $mediatype) {
    $accountId = getAccountIdByUsername($username);
    if (!$accountId) {
        return array('success' => false, 'message' => 'Username not found.');
    }
    $insertConn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    $deleteConn = new mysqli('10.244.1.2', 'DeleteAccessOnly', 'cl3v3rm@ndoesTh1ngs!', 'SceneSync');
    $status = [];
    if ($insertConn->connect_error || $deleteConn->connect_error) {
        $status['success'] = false;
        $status['message'] = "Connection failed: " . $insertConn->connect_error . " | " . $deleteConn->connect_error;
    } else {
        $watchedSql = "INSERT INTO WatchedList (AccountID, MovieID, Movie, PosterURL, Year, MediaType, TimeCreated) VALUES (?, ?, ?, ?, ?, ?, NOW())";
        $watchedStmt = $insertConn->prepare($watchedSql);
        $watchedStmt->bind_param("isssss", $accountId, $MoviesID, $movieTitle, $posterURL, $year, $mediatype);
        if ($watchedStmt->execute()) {
            $movieID = $watchedStmt->insert_id; // Retrieve the last inserted ID
            $status['success'] = true;
            $status['message'] = "Added to Watched List successfully. MovieID: $movieID";
            $deleteSql = "DELETE FROM WatchList WHERE AccountID = ? AND Movie = ?";
            $deleteStmt = $deleteConn->prepare($deleteSql);
            $deleteStmt->bind_param("is", $accountId, $movieTitle);
            if ($deleteStmt->execute()) {
                $status['message'] .= " Deleted from Watch List successfully.";
            } else {
                $status['success'] = false;
                $status['message'] = "Error deleting from Watch List: " . $deleteStmt->error;
            }
            $deleteStmt->close();
        } else {
            $status['success'] = false;
            $status['message'] = "Error adding to Watched List: " . $watchedStmt->error;
        }
        $watchedStmt->close();
    }
    $insertConn->close();
    $deleteConn->close();
   // sendStatusToRabbitMQ(json_encode($status));
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
        $accountId = $row['AccountID'];
        $conn->close();
        return $accountId;
    } else {
        $conn->close();
        return false; // Username not found
    }
}

//isfar
function calculateRateScoreAndLeaderboard() {
    if ($connection->connect_error) {
        die("Connection failed: " . $connection->connect_error);
    }
    $query = "SELECT Profiles.RateScore, Profiles.AccountID, Accounts.Username 
              FROM Profiles 
              JOIN Accounts ON Accounts.AccountID = Profiles.AccountID 
              ORDER BY RateScore DESC 
              LIMIT 10";
    $result = $connection->query($query);
    if (!$result) {
        die('Error in query: ' . $connection->error);
    }
    $leaderboard = [];
    while ($row = $result->fetch_assoc()) {
        $leaderboard[] = $row;
    }
    $result->free_result();
    return $leaderboard;
}




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

function insertReview($accountId, $MovieID, $movieTitle, $rating, $review){
    $connection = dbConnect();
    $stmt = $connection->prepare("INSERT INTO Ratings(AccountID, MovieID, Movie, Rating, Review) VALUES($accountId, $movieTitle, $rating, $review)");
    $stmt->bind_param("dsds", $accountId, $MovieID, $movieTitle, $rating, $review);
    $stmt->execute();
    $succeeded = $stmt->errno === 0;
    $stmt->close();
    $connection->close();
    http_response_code($succeeded ? 200 : 500);
    return json_encode(["returnCode" => $succeeded ? 1 : 0]);
}

function searchRatingsByMovie($movieTitle){
    $result = [];
    $connection = dbConnect();
    $stmt = $connection->prepare("SELECT AccountID, Movie, Rating, Review FROM Ratings WHERE Movie = ?");
    $stmt->bind_param("s", $movieTitle);
    $stmt->execute();
    $stmt->bind_result($accountId, $movie, $rating, $review);
    while($stmt->fetch()){
        $result[] = [
            "AccountID" => $accountId,
            "movieTitle" => $movie,
            "Rating" => $rating,
            "Review" => $review
        ];
    }
    $stmt->close();
    $connection->close();
    http_response_code(200);
    return json_encode(["returnCode" => '1', "reviews" => $result]);
}
function updateRateScore($username, $value) {
    $mysqli = dbConnect();
    $accountID = getAccountIdByUsername($username);
    if (!$accountID) {
        return false;
    }
    $query = "SELECT RateScore FROM Profiles WHERE AccountID = '$accountID'";
    $result = $mysqli->query($query);
    $row = $result->fetch_assoc();
    $rateScore = $row['RateScore'];
    $newRateScore = $rateScore + $value;
    $updateQuery = "UPDATE Profiles SET RateScore = '$newRateScore' WHERE AccountID = '$accountID'";
    if ($mysqli->query($updateQuery) === TRUE) {
        return true;
    }
    return false;

//isfar
function resetPassword($username, $email, $assword) {
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    $query = "SELECT * FROM Users WHERE Username = ? AND Email = ?";
    $statement = $connection->prepare($query);
    if (!$statement) {
        return ['success' => false, 'message' => 'Error in query preparation: ' . $connection->error];
    }
    $statement->bind_param('ss', $username, $email);
    $statement->execute();
    $result = $statement->get_result();
    if ($result->num_rows == 0) {
        return ['success' => false, 'message' => 'Invalid username or email.'];
    }
//JAKEEEEEEE DO STUFF HERE
    $updateQuery = "UPDATE Users SET Password = ? WHERE Username = ?";
    $updateStatement = $connection->prepare($updateQuery);
    if (!$updateStatement) {
        return ['success' => false, 'message' => 'Error in query preparation: ' . $connection->error];
    }
    $updateStatement->bind_param('ss', $password, $username);
    $updateResult = $updateStatement->execute();
    if ($updateResult) {
        return ['success' => true, 'message' => 'Password reset successfully.'];
    } else {
        return ['success' => false, 'message' => 'Error resetting password: ' . $updateStatement->error];
    }
    $statement->close();
    $updateStatement->close();
    $result->close();
}

}
?>