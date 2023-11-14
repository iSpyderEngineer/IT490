#!/usr/bin/php
<?php
require_once('rabbitFunctions.inc');
require_once('../Backend/dbConnection.inc');
require_once('../Backend/dbFunctions.inc');

function requestProcessor($request) {
    $response = '';
    echo "received request" . PHP_EOL;
    var_dump($request);

    if (!isset($request['type'])) {
        return "ERROR: unsupported message type";
    }

    switch ($request['type']) {
        case "login":
            echo "logging in";
            return validateLogin($request['username'], $request['password']);

        case "register":
            echo "registering now";
            return validateRegister($request['firstname'], $request['lastname'], $request['username'], $request['email'], $request['address'], $request['city'], $request['country'], $request['zipcode'], $request['password']);

        case "getUserProfile":
            echo "getting user profile";
            return getUserProfileData($request['sessionID']);

        case "updateProfile":
            echo "updating profile settings";
            return updateProfileSettings($request['sessionID'], $request['data']);

        case "getWatchList":
            echo "getting watch list data";
            return getWatchListData($request['accountId']);

        case "getWatchedList":
            echo "getting watched list data";
            return getWatchedListData($request['accountId']);

        case "addToWatchList":
            echo "adding to watch list";
            return addToWatchList($request['accountId'], $request['title'], $request['year'], $request['isMovie'], $request['isTV']);

        case "addToWatchedList":
            echo "adding to watched list";
            return addToWatchedList($request['accountId'], $request['title'], $request['year'], $request['isMovie'], $request['isTV']);
    }

    return json_encode(array("returnCode" => '0', 'message' => "Server message received but type not defined"));
}

$server = new rabbitMQServer("testRabbitMQ.ini", "database");
echo "db server started" . PHP_EOL;
$server->process_requests('requestProcessor');
exit();
?>
