<?php
require_once('dbConnection.inc');
require_once('rabbitFunctions.inc');
require_once('SessionID.inc');

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

function getUserProfileData($sessionID) {
    $mysqli = dbConnect(); 
    $query = "SELECT FavoriteDirector, FavoriteGenre, FavoriteMovie, FavoriteActor, Bio FROM Users WHERE SessionID = ?";
    
    if ($stmt = $mysqli->prepare($query)) {
        $stmt->bind_param("s", $sessionID);
        $stmt->execute();
        $stmt->bind_result($favoriteDirector, $favoriteGenre, $favoriteMovie, $favoriteActor, $bio);
        $stmt->fetch();
        $stmt->close();

        $userData = [
            "FavoriteDirector" => $favoriteDirector,
            "FavoriteGenre" => $favoriteGenre,
            "FavoriteMovie" => $favoriteMovie,
            "FavoriteActor" => $favoriteActor,
            "Bio" => $bio,
        ];

        return $userData;
    }

    return null; 
}

function updateProfileSettings($username, $favActor, $favGenre, $favDirector, $favMovie, $biography) {
    $mysqli = dbConnect(); 
    $AccountID - getAccountID($username);
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
function getWatchListData($accountId) {
    $conn = new mysqli(10.244.1.2, 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');

    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }

    $sql = "SELECT * FROM WatchList WHERE AccountID = ?";
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("i", $accountId);
    $stmt->execute();

    $result = $stmt->get_result();

    $watchListData = array();
    while ($row = $result->fetch_assoc()) {
        $watchListData[] = $row;
    }

    $stmt->close();
    $conn->close();

    return $watchListData;
}
function getWatchedListData($accountId) {
    $conn = new mysqli(10.244.1.2, 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');

    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }

    $sql = "SELECT * FROM WatchedList WHERE AccountID = ?";
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("i", $accountId);
    $stmt->execute();

    $result = $stmt->get_result();

    $watchListData = array();
    while ($row = $result->fetch_assoc()) {
        $watchListData[] = $row;
    }

    $stmt->close();
    $conn->close();

    return $watchListData;
}
function addToWatchList($accountId, $movie, $year) {
    global $servername, $username, $password, $dbname;

    $conn = new mysqli(10.244.1.2, 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');

    $status = []; 

    if ($conn->connect_error) {
        $status['success'] = false;
        $status['message'] = "Connection failed: " . $conn->connect_error;
    } else {
        $sql = "INSERT INTO WatchList (AccountID, Movie, Year, TimeCreated, LastModified) VALUES (?, ?, ?, ?, ?, NOW(), NOW())";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("isiii", $accountId, $movie, $year);

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


function addToWatchedList($accountId, $movie, $year) {
    global $servername, $username, $password, $dbname;
    $conn = new mysqli(10.244.1.2, 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    $sql = "INSERT INTO WatchList (AccountID, Movie, Year, TimeCreated, LastModified) VALUES (?, ?, ?, ?, ?, NOW(), NOW())";
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("isiii", $accountId, $movie, $year);
    $stmt->execute();
    $stmt->close();
    $conn->close();
    if ($stmt->execute()) {
        $status['success'] = true;
        $status['message'] = "Added successfully.";
    } else {
        $status['success'] = false;
        $status['message'] = "Error: " . $stmt->error;
    }
    $stmt->close();
$conn->close();
sendStatusToRabbitMQ(json_encode($status));
return $status;
}


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

function addUserBioToDatabase($userId, $userBio) {
    $conn = new mysqli(10.244.1.2, 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    $userId = $conn->real_escape_string($userId);
    $userBio = $conn->real_escape_string($userBio);
   // update sql statement $sql = "UPDATE users SET bio = '$userBio' WHERE id = '$userId'";
    if ($conn->query($sql) === TRUE) {
        echo "User bio added successfully";
    } else {
        echo "Error updating user bio: " . $conn->error;
    }
    $conn->close();
}










?>
