#!/usr/bin/php
<?php
// Include necessary files for RabbitMQ server and API functions
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');
require_once('../Backend/apiFunctions.inc');

// Function to process incoming requests
function requestProcessor($request){
  // Log the received request
  echo "received request" . PHP_EOL;
  var_dump($request);

  // Check if the 'type' field is set in the request
  if (!isset($request['type'])) {
    // Return an error if 'type' is missing
    return ['status' => 'error', 'message' => 'Request type is missing'];
  }

  // Handle different request types
  switch ($request['type']) {
    case "searchMoviesAndTVShows":
      // Handle movie and TV show search requests
      if (isset($request['query'])) {
        $response = searchMoviesandTVShows($request['query']);
        var_dump($response);
        return $response;
      } else {
        // Return an error if 'query' parameter is missing
        return ['status' => 'error', 'message' => 'Query parameter is missing'];
      }

    case "searchPerson":
      // Handle person search requests
      if (isset($request['personName'])) {
        $response = searchPerson($request['personName']);
        var_dump($response);
        return $response;
      } else {
        // Return an error if 'personName' parameter is missing
        return ['status' => 'error', 'message' => 'Person name parameter is missing'];
      }

    case "recommendationActorDirector":
      // Handle actor/director recommendation requests
      if (isset($request['username'])) {
        $response = recommendationActorDirector($request['username']);
        var_dump($response);
        return $response;
      } else {
        // Return an error if 'username' parameter is missing
        return ['status' => 'error', 'message' => 'Username parameter is missing'];
      }

    case "getMoviesByActor":
      // Handle requests to get movies by actor
      if (isset($request['actorName'])) {
        $response = getMoviesByActor($request['actorName']);
        var_dump($response);
        return $response;
      } else {
        // Return an error if 'actorName' parameter is missing
        return ['status' => 'error', 'message' => 'Actor name parameter is missing'];
      }

    case "getMoviesByDirector":
      // Handle requests to get movies by director
      if (isset($request['directorName'])) {
        $response = getMoviesByDirector($request['directorName']);
        var_dump($response);
        return $response;
      } else {
        // Return an error if 'directorName' parameter is missing
        return ['status' => 'error', 'message' => 'Director name parameter is missing'];
      }

    case "getMoviesByMovieAndGenre":
      // Handle requests to get movies by movie and genre
      if (isset($request['username'])) {
        $response = getMoviesByMovieAndGenre($request['username']);
        var_dump($response);
        return $response;
      } else {
        // Return an error if 'username' parameter is missing
        return ['status' => 'error', 'message' => 'Username parameter is missing'];
      }

    case "getMoviesByDetails":
      // Handle requests to get movie details
      if (isset($request['movieID'])) {
        $response = getMoviesByDetails($request['movieID']);
        var_dump($response);
        return $response;
      } else {
        // Return an error if 'movieID' parameter is missing
        return ['status' => 'error', 'message' => 'Movie ID parameter is missing'];
      }

    case "getTVByDetails":
      // Handle requests to get tv details
      if (isset($request['tvID'])) {
        $response = getTVByDetails($request['tvID']);
        var_dump($response);
        return $response;
      } else {
        // Return an error if 'tvID' parameter is missing
        return ['status' => 'error', 'message' => 'Movie ID parameter is missing'];
      }
  
    case "getRecentWatchedRecommendations":
      // Handle requests for recent watched recommendations
      if (isset($request['username'])) {
          $response = getRecentWatchedRecommendations($request['username']);
          var_dump($response);
          return $response;
      } else {
          // Return an error if 'username' parameter is missing
          return ['status' => 'error', 'message' => 'Username parameter is missing'];
      }

    case "getMostRecentWatched":
        // Handle requests to get the most recent watched movie
        if (isset($request['username'])) {
            $response = getMostRecentWatched($request['username']);
            var_dump($response);
            return $response;
        } else {
            // Return an error if 'username' parameter is missing
            return ['status' => 'error', 'message' => 'Username parameter is missing'];
        }


    default:
      // Handle unsupported request types
      echo "Request type not handled\n";
      return ['status' => 'error', 'message' => "Request type '{$request['type']}' not supported"];
  }
}

// Initialize RabbitMQ server
$server = new rabbitMQServer("testRabbitMQ.ini","api");

// Start the API server
echo "api server started up" . PHP_EOL;
// Process incoming requests using the 'requestProcessor' function
$server->process_requests('requestProcessor');
// Log when the server is shut down
echo "api server shut down" . PHP_EOL;
exit();
?>