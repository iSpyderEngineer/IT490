<?php
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');

function sendDB($message) {
	$client = new rabbitMQClient('testRabbitMQ.ini','database');
	$response = $client -> send_request($message);
	return $response;
}

function sendAPI($message) {
	$client = new rabbitMQClient('testRabbitMQ.ini','api');
	$response = $client -> send_request($message);
	return $response;
}
?>