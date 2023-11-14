<?php
require_once(__DIR__ . '/server/path.inc');
require_once(__DIR__ . '/server/get_host_info.inc');
require_once(__DIR__ . '/server/rabbitMQLib.inc');

if ($_POST) {
  $request = array();
  $request['type'] = $_POST["type"];

  if ($request['type'] === "displayRecommendedMovies") {
    $request['movieData'] = $_POST['movieData'];
    $request['source'] = $_POST['source'];
    
  } elseif ($request['type'] === "searchMovies") {
    $request['query'] = $_POST['query'];

  } elseif ($request['type'] === "fetchUserProfile") {
    $request['sessionID'] = $_POST['sessionID'];

  } elseif ($request['type'] === "updateUserPreferences") {
    $request['userID'] = $_POST['userID'];
    $request['preferences'] = $_POST['preferences'];
  } else {
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
