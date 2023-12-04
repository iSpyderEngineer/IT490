<?php
// Include necessary files for RabbitMQ connection
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');

// Check if there is a POST request
if ($_POST) {
    // Initialize an array to store the request data and set the 'type' of request based on POST data
    $request = array();
    $request['type'] = $_POST["type"];
    
    // Switch case to handle different request types
    switch ($request['type']) {
        // Handle display recommended movies request
        case "displayRecommendedMovies":
            $request['movieData'] = $_POST['movieData'];
            $request['source'] = $_POST['source'];
            break;

        // Handle search movies request
        case "searchMovies":
            $request['query'] = $_POST['query'];
            break;

        // Handle update user preferences request
        case "updateUserPreferences":
            $request['userID'] = $_POST['userID'];
            $request['preferences'] = $_POST['preferences'];
            break;

        // Handle search movies and TV shows request
        case "searchMoviesAndTVShows":
            $request['query'] = $_POST['query'];
            break;

        // Handle search person request
        case "searchPerson":
            $request['personName'] = $_POST['personName'];
            break;

        // Handle recommendation for actor or director request
        case "recommendationActorDirector":
            $request['username'] = $_POST['username'];
            break;

        // Handle get movies by actor request
        case "getMoviesByActor":
            $request['actorName'] = $_POST['actorName'];
            break;

        // Handle get movies by director request
        case "getMoviesByDirector":
            $request['directorName'] = $_POST['directorName'];
            break;

        // Handle get movies by movie and genre request
        case "getMoviesByMovieAndGenre":
            $request['username'] = $_POST['username'];
            break;

        // Handle get movie by details request
        case "getMovieByDetails":
            $request['movieID'] = $_POST['movieID'];
            break;
        
        // Handle get recent watched recommendations 
        case "getRecentWatchedRecommendations":
            $request['username'] = $_POST['username'];
            break;

        // Handle get most recent watched movie request
        case "getMostRecentWatched":
            $request['username'] = $_POST['username'];
            break;

        // Default case for invalid request types
        default:
            echo json_encode(["error" => "Invalid request type"]);
            exit;
    }

    // Create a new RabbitMQ client instance, send the request, and output the response
    $client = new rabbitMQClient("testRabbitMQ.ini", "api");
    $response = $client->send_request($request);
    header("Content-Type: application/json");
    echo json_encode($response);

    if (is_array($response)) {
        $response = json_encode($response);
    }

} else {
    // Output an error message if no POST data is received
    echo json_encode(["error" => "No POST data received"]);
}