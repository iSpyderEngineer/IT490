<?php
// getUserInfo.php

// Start the session (if not started already)
session_start();

// Check if the user is logged in (username is in the session)
if (isset($_SESSION['username'])) {
    $username = $_SESSION['username'];

    // Here, you can perform any actions or return data based on the logged-in user
    // For example, you can return the username as a JSON response
    echo json_encode(['username' => $username]);
} else {
    // If the user is not logged in or username is not set in the session
    echo json_encode(['error' => 'User not logged in']);
}
?>
