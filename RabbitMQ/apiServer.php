#!/usr/bin/php
<?php
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');
require_once('../Backend/apiFunctions.inc');
require_once('rabbitFunctions.inc');

function requestProcessor($request){
  echo "received request" . PHP_EOL;
  var_dump($request);
  switch ($request['type']) {
    case "displayRecommendedMovies":
      echo "Displaying recommended movies";
      return displayRecommendedMovies($request['movieData'], $request['source']);

    case "searchMovies":
      echo "Searching for movies";
      return searchMoviesAndTVShows($request['query']);

    case "fetchUserProfile":
      echo "Fetching user profile";
      return fetchUserProfile($request['sessionID']);

    case "updateUserPreferences":
      echo "Updating user preferences";
      return updateUserPreferences($request['userID'], $request['preferences']);

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
