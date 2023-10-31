<?php
require_once(__DIR__ . '/server/path.inc');
require_once(__DIR__ . '/server/get_host_info.inc');
require_once(__DIR__ . '/server/rabbitMQLib.inc');

if ($_POST){
  $request = array();
  $password = hash("sha256", $_POST["password"]);
  $request['type'] = $_POST["type"];
  $request['username'] = $_POST["username"];
  $request['password'] = $password;

  $client = new rabbitMQClient("testRabbitMQ.ini","database");
  if (isset($_POST["email"])){
	  $request["email"] = $_POST["email"];
  }
  
  $response = $client -> send_request($request);
  echo $response;
} else {
	$error = array();
	$error["message"] = "error";
	echo json_encode($r);
}
?>