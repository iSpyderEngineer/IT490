<?php
require_once(__DIR__ . '/server/path.inc');
require_once(__DIR__ . '/server/get_host_info.inc');
require_once(__DIR__ . '/server/rabbitMQLib.inc');

if ($_POST) {
    $request = array();
    $request['type'] = $_POST["type"];
    
    switch ($request['type']) {
        case "login":
            $request['username'] = $_POST['username'];
            $request['password'] = $_POST['password'];
            break;

        case "register":
            $request['firstname'] = $_POST['firstname'];
            $request['lastname'] = $_POST['lastname'];
            $request['username'] = $_POST['username'];
            $request['email'] = $_POST['email'];
            $request['address'] = $_POST['address'];
            $request['city'] = $_POST['city'];
            $request['country'] = $_POST['country'];
            $request['zipcode'] = $_POST['zipcode'];
            $request['password'] = $_POST['password'];
            break;

        case "getUserProfile":
            $request['username'] = $_POST['username'];
            break;

        case "updateProfile":
            $request['username'] = $_POST['username'];
            $request['favActor'] = $_POST['favActor'];
            $request['favGenre'] = $_POST['favGenre'];
            $request['favDirector'] = $_POST['favDirector'];
            $request['favMovie'] = $_POST['favMovie'];
            $request['biography'] = $_POST['biography'];
            break;

        case "getWatchList":
        case "getWatchedList":
        case "addToWatchList":
        case "addToWatchedList":
            $request['username'] = $_POST['username'];
            $request['movieTitle'] = $_POST['movieTitle'];
            $request['posterURL'] = $_POST['posterURL'];
            $request['year'] = $_POST['year'];
            break;

        default:
            echo json_encode(["error" => "Invalid request type"]);
            exit;
    }

    $client = new rabbitMQClient("testRabbitMQ.ini", "api");
    $response = $client->send_request($request);
    echo $response;

} else {
    echo json_encode(["error" => "No POST data received"]);
}
?>
