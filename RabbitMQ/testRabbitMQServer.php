#!/usr/bin/php
<?php
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');

function processMessage($request)
{
    echo "Received request: " . json_encode($request) . "\n";
    $response = ['status' => 'success', 'message' => 'Message processed'];
    return $response;
}

try {
    $server = new rabbitMQServer("testRabbitMQ.ini","test");
    $server->process_requests('processMessage');
} catch (Exception $e) {
    error_log("Server error: " . $e->getMessage());
}
?>