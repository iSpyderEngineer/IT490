<?php

function validateSession($sessionID) {
    // session validation code here 
   
    return true; 
}

function processRequest() {
    if (isset($_POST['sessionid'])) {
        $sessionid = $_POST['sessionid'];
        
        if (validateSession($sessionid)) {
            $response = array('returnCode' => '1');
            echo json_encode($response);
        } else {
            $response = array('returnCode' => '0');
            echo json_encode($response);
        }
    } else {

        // Return error + sent to login page maybe??
    }
}

processRequest();
?>

