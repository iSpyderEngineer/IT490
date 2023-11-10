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

function updateProfileSettings($sessionID, $data) {
    $mysqli = dbConnect(); 

    $updateQuery = "UPDATE Users SET FavoriteDirector = ?, FavoriteGenre = ?, FavoriteMovie = ?, FavoriteActor = ?, Bio = ? WHERE SessionID = ?";
    
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




?>
