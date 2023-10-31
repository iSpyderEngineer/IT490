<?php
require_once('dbConnect.php');
function dbConnect() {
    $mysqli = dbConnect(); 
}

function validateLogin($username, $password) {
    $mysqli = dbConnect(); 
    $query = "SELECT PasswordHash FROM Accounts WHERE username = ?";
    
    if ($stmt = $mysqli->prepare($query)) {
        $stmt->bind_param("s", $username);
        $stmt->execute();
        $stmt->bind_result($storedPassword);
        $stmt->fetch();
        $stmt->close();
    }

    if (empty($storedPassword) || !password_verify($password, $storedPassword)) {
        return ["status" => 401, "message" => "Invalid credentials"];
    }

    return ["status" => 200, "message" => "Login successful"];
}

if (isset($_SERVER['REQUEST_METHOD']) && $_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents('php://input'), true);
    
    if (!isset($data['username']) || !isset($data['password'])) {
        http_response_code(400);
        //echo json_encode(["message" => "Missing username or password"]);
        return json_encode(array("returnCode" => '0', 'message' => "Username already exists, try again"));
        exit;
    }

    $username = $data['username'];
    $password = $data['password'];

    $result = validateLogin($username, $password, $mysqli);

    http_response_code($result['status']);
    //echo json_encode(["message" => $result['message']]);
    return json_encode(array("returnCode" => '1', 'message' => "Success!"));
}

$mysqli->close();
?>
