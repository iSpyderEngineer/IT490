#!/usr/bin/php
<?php
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');
require_once('../Backend/apiFunctions.inc');

function requestProcessor($request){
  echo "received request" . PHP_EOL;
  var_dump($request);
  switch ($request['type']) {
    case "searchMovies":
      echo "Searching for movies";
      return searchMoviesAndTVShows($request['query']);

    case "searchMoviesAndTVShows":
      echo "Searching for tv shows and movies";
      return searchMoviesandTVShows($request['query']);

    case "searchPerson":
      echo "Searching for a person";
      return searchPerson($request['personName']);

    case "recommendationActorDirector":
      echo "Getting recommendations based on actor and director";
      return recommendationActorDirector($request['username']);

    case "getMoviesByActor":
      echo "Getting movies by actor";
      return getMoviesByActor($request['actorName']);

    case "getMoviesByDirector":
      echo "Getting movies by director";
      return getMoviesByDirector($request['directorName']);

    case "getMoviesByMovieAndGenre":
      echo "Getting movies by movie and genre";
      return getMoviesByMovieAndGenre($request['username']);

    case "getMoviesByDetails":
      echo "Getting details for movie";
      return getMoviesByDetails($request['movieID']);

    default:
      echo "Request type not handled";
      return ["error" => "Request type not supported"];
  }
}

$server = new rabbitMQServer("testRabbitMQ.ini","api");

echo "api server started up" . PHP_EOL;
$server -> process_requests('requestProcessor');
echo "api server shut down" . PHP_EOL;
exit();
?>