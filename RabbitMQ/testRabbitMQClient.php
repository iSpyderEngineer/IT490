#!/usr/bin/php
<?php
require_once('rabbitMQLib.inc');
require_once('path.inc');
require_once('get_host_info.inc');

try {
    $client = new rabbitMQClient("testRabbitMQ.ini", "test");
    $testMessage = ['type' => 'test', 'content' => 'Hello, server!'];
    $response = $client->send_request($testMessage);
    echo "Server response: " . json_encode($response) . "\n";
} catch (Exception $e) {
    error_log("Client error: " . $e->getMessage());
}
?>