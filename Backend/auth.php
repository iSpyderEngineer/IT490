<?php

// database connection
require_once('dbConnect.php');

if (isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents('php://input'), true);
    
    if (!isset($data['username']) || !isset($data['password'])) {
        http_response_code(400);
        echo json_encode(["message" => "Missing username or password"]);
        exit;
    }

    $username = $data['username'];
    $password = $data['password'];

    // Code for database to retrieve the hashed password for the username
    $query = "SELECT PasswordHash FROM Accounts WHERE username = ?";
    
    if ($stmt = $mysqli->prepare($query)) {
        $stmt->bind_param("s", $username);
        $stmt->execute();
        $stmt->bind_result($storedPassword);
        $stmt->fetch();
        $stmt->close();
    }

    if (empty($storedPassword) || !password_verify($password, $storedPassword)) {
        http_response_code(401);
        echo json_encode(["message" => "Invalid credentials"]);
        exit;
    }

    // Authentication successful
    http_response_code(200);
    echo json_encode(["message" => "Login successful"]);
}
$mysqli->close();
