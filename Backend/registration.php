<?php
require_once('dbConnect.php');

function dbConnect() {
    global $mysqli;
    return $mysqli;
}

function registerUser($data) {
    $mysqli = dbConnect();

    $requiredFields = ['FirstName', 'LastName', 'Username', 'Email', 'Address', 'City', 'Country', 'ZipCode', 'PasswordHash'];
    foreach ($requiredFields as $field) {
        if (!isset($data[$field]) || empty($data[$field])) {
            http_response_code(400);
            echo json_encode(["message" => "Missing or empty field: $field"]);
            exit;
        }
    }

    $firstName = $data['FirstName'];
    $lastName = $data['LastName'];
    $username = $data['Username'];
    $email = $data['Email'];
    $address = $data['Address'];
    $city = $data['City'];
    $country = $data['Country'];
    $zipCode = $data['ZipCode'];
    $passwordHash = password_hash($data['PasswordHash'], PASSWORD_BCRYPT);

    // Check if the username or email is already in the database
    $query = "SELECT id FROM Accounts WHERE Username = ? OR Email = ?";
    if ($stmt = $mysqli->prepare($query)) {
        $stmt->bind_param("ss", $username, $email);
        $stmt->execute();
        $stmt->store_result();

        if ($stmt->num_rows > 0) {
            http_response_code(400);
            echo json_encode(["message" => "Username or email already in use"]);
            exit;
        }

        $stmt->close();
    }

    // Insert the data into the database
    $insertQuery = "INSERT INTO Accounts (FirstName, LastName, Username, Email, Address, City, Country, ZipCode, PasswordHash) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
    if ($insertStmt = $mysqli->prepare($insertQuery)) {
        $insertStmt->bind_param("sssssssss", $firstName, $lastName, $username, $email, $address, $city, $country, $zipCode, $passwordHash);
        if ($insertStmt->execute()) {
            http_response_code(201); // Created
            //echo json_encode(["message" => "User registered successfully"]);
            return json_encode(array("returnCode" => '1', 'message' => "User registered successfully Username already exists, try again"));
        } else {
            http_response_code(500);
            //echo json_encode(["message" => "Failed to register user"]);
            return json_encode(array("returnCode" => '0', 'message' => "Username already exists, try again"));
        }
        $insertStmt->close();
    }
}

if (isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents('php://input'), true);

    if (!isset($data['Username']) || !isset($data['PasswordHash'])) {
        http_response_code(400);
        echo json_encode(["message" => "Missing username or password"]);
        exit;
    }

    registerUser($data);
}

$mysqli->close();
?>
