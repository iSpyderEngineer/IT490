
<?php
// Database connection
$host = '10.244.1.2';
$username = 'BackEndAdmin';
$password = 'Qg5OKQ!?$Q';
$database = 'SceneSync';

$mysqli = new mysqli($host, $username, $password, $database);
echo "database connected
";
// Check for errors
if ($mysqli->connect_error) {
    die("Database connection failed: " . $mysqli->connect_error);
}
