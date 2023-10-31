<?php
// Database connection
$host = '10.244.1.2';
$username = 'BackEndAdmin';
$password = 'Qg5OKQ!?$Q';
$database = 'SceneSync';

$conn = new mysqli($host, $username, $password, $database);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$sql = "INSERT INTO Accounts (firstName, LastName, Username, Email, Address, City, Country, ZipCode, PasswordHash) VALUES ('Tester', 'One', 'test4', 'test1@testing.com', '801 Testing Street', 'Test City', 'USA', '12345', 'abcdefghijklmnop')";


//$result = $conn->query($sql);




if ($conn->query($sql) ===TRUE) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();


?>