<?php
require_once('RabbitMQ/path.inc');
require_once('RabbitMQ/get_host_info.inc');
require_once('RabbitMQ/rabbitMQLib.inc');

$client = new rabbitMQClient("testRabbitMQ.ini","database");

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['message'])) {
    $message = $_POST['message'];

    $request = array();
    $request['type'] = "Login";
    $request['username'] = "steve";
    $request['password'] = "password";
    $request['message'] = $message;

    // Send the request and wait for a response
    $response = $client->send_request($request);

    // Handle the response (you can display it or process it further)
    echo "Response from server: " . $response . PHP_EOL;
}
?>

<!DOCTYPE html>
<html>
<body>
    <h1>RabbitMQ Message Sender</h1>
    <form method="post" action="">
        <label for="message">Message:</label>
        <input type="text" id="message" name="message" required>
        <button type="submit">Send Message</button>
    </form>
</body>
</html>
