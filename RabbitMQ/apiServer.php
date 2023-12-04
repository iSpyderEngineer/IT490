#!/usr/bin/php
<?php
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');
require_once('../Backend/apiFunctions.inc'); // Ensure correct path

function requestProcessor($request){
  echo "received request" . PHP_EOL;
  var_dump($request);

  if (!isset($request['type'])) {
    return ['status' => 'error', 'message' => 'Request type is missing'];
  }

  switch ($request['type']) {
    case "searchMoviesAndTVShows":
      if (isset($request['query'])) {
        $response = searchMoviesandTVShows($request['query']);
        var_dump($response);
        return $response;
      } else {
        return ['status' => 'error', 'message' => 'Query parameter is missing'];
      }

    case "searchPerson":
      if (isset($request['personName'])) {
        $response = searchPerson($request['personName']);
        var_dump($response);
        return $response;
      } else {
        return ['status' => 'error', 'message' => 'Person name parameter is missing'];
      }

    case "recommendationActorDirector":
      if (isset($request['username'])) {
        $response = recommendationActorDirector($request['username']);
        var_dump($response);
        return $response;
      } else {
        return ['status' => 'error', 'message' => 'Username parameter is missing'];
      }

    case "getMoviesByActor":
      if (isset($request['actorName'])) {
        $response = getMoviesByActor($request['actorName']);
        var_dump($response);
        return $response;
      } else {
        return ['status' => 'error', 'message' => 'Actor name parameter is missing'];
      }

    case "getMoviesByDirector":
      if (isset($request['directorName'])) {
        $response = getMoviesByDirector($request['directorName']);
        var_dump($response);
        return $response;
      } else {
        return ['status' => 'error', 'message' => 'Director name parameter is missing'];
      }

    case "getMoviesByMovieAndGenre":
      if (isset($request['username'])) {
        $response = getMoviesByMovieAndGenre($request['username']);
        var_dump($response);
        return $response;
      } else {
        return ['status' => 'error', 'message' => 'Username parameter is missing'];
      }

    case "getMoviesByDetails":
      if (isset($request['movieID'])) {
        $response = getMoviesByDetails($request['movieID']);
        var_dump($response);
        return $response;
      } else {
        return ['status' => 'error', 'message' => 'Movie ID parameter is missing'];
      }

    default:
      echo "Request type not handled\n";
      return ['status' => 'error', 'message' => "Request type '{$request['type']}' not supported"];
  }
}

$server = new rabbitMQServer("testRabbitMQ.ini","api");

echo "api server started up" . PHP_EOL;
$server->process_requests('requestProcessor');
echo "api server shut down" . PHP_EOL;
exit();
?>