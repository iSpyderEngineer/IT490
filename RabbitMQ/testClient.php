<?php
require_once(__DIR__ . '/server/path.inc');
require_once(__DIR__ . '/server/get_host_info.inc');
require_once(__DIR__ . '/server/rabbitMQLib.inc');

if ($_POST){
  $request = array();
  $request["type"] = $_POST["type"];
  $request["message"] = $_POST["message"];

  $client = new rabbitMQClient("testRabbitMQ.ini","database");
  
  $response = $client -> send_request($request);
  echo $response;
} else {
	$error = array();
	$error["message"] = "error";
	echo json_encode($r);
}
?>