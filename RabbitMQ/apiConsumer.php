<?php
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');

if ($_POST) {
    $request = array();
    $request['type'] = $_POST["type"];
    
    switch ($request['type']) {
        case "displayRecommendedMovies":
            $request['movieData'] = $_POST['movieData'];
            $request['source'] = $_POST['source'];
            break;

        case "searchMovies":
            $request['query'] = $_POST['query'];
            break;

        case "updateUserPreferences":
            $request['userID'] = $_POST['userID'];
            $request['preferences'] = $_POST['preferences'];
            break;

        case "searchMoviesAndTVShows":
            $request['query'] = $_POST['query'];
            break;

        case "searchPerson":
            $request['personName'] = $_POST['personName'];
            break;

        case "recommendationActorDirector":
            $request['username'] = $_POST['username'];
            break;

        case "getMoviesByActor":
            $request['actorName'] = $_POST['actorName'];
            break;

        case "getMoviesByDirector":
            $request['directorName'] = $_POST['directorName'];
            break;

        case "getMoviesByMovieAndGenre":
            $request['username'] = $_POST['username'];
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
