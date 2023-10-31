#!/usr/bin/php
<?php
require_once('rabbitFunctions.inc');
require_once('../Database/dbConnection.inc');
require_once('../Backend/dbFunctions.inc');

function requestProcessor($request) {
  $response = '';
  echo "received request" . PHP_EOL;
  var_dump($request);

  if(!isset($request['type'])){
    return "ERROR: unsupported message type";
  }
  switch ($request['type']){
    case "testRabbit":
      $request["message"] = "sent";
      return $request["message"];
  }
  return json_encode(array("returnCode" => '0', 'message'=>"Server message recieved but type not defined"));
}


$server = new rabbitMQServer("testRabbitMQ.ini", "database");
echo "db server started" . PHP_EOL;
$server -> process_requests('requestProcessor');
exit();
?>