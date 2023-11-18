#!/usr/bin/php
<?php
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');

function processMessage($msg)
{
   echo "Received a message:\n";
   print_r($msg);

   $response = ['status' => 'success', 'content' => 'Message processed'];
   return $response;
}

$server = new rabbitMQServer("testRabbitMQ.ini", "database");

$server->process_requests('processMessage');

echo "Server is running...\n";
?>



