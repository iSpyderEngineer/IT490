<?php

require_once('get_host_info.inc');

// Class for managing RabbitMQ Server
class rabbitMQServer
{
    // Private and public variables for RabbitMQ connection configuration
    private $machine = "";
    public $BROKER_HOSTS;
    private $BROKER_PORT;
    private $USER;
    private $PASSWORD;
    private $VHOST;
    private $exchange;
    private $queue;
    private $routing_key = '*';
    private $exchange_type = "topic";
    private $auto_delete = false;

    // Constructor to initialize server settings
    function __construct($machine, $server = "rabbitMQ")
    {
        // Retrieve and set server configurations from the provided machine info
        $this->machine = getHostInfo(array($machine));
        $this->BROKER_HOSTS = $this->machine[$server]["BROKER_HOSTS"];
        $this->BROKER_PORT = $this->machine[$server]["BROKER_PORT"];
        $this->USER = $this->machine[$server]["USER"];
        $this->PASSWORD = $this->machine[$server]["PASSWORD"];
        $this->VHOST = $this->machine[$server]["VHOST"];

        // Optional settings for exchange type and auto delete
        if (isset($this->machine[$server]["EXCHANGE_TYPE"])) {
            $this->exchange_type = $this->machine[$server]["EXCHANGE_TYPE"];
        }
        if (isset($this->machine[$server]["AUTO_DELETE"])) {
            $this->auto_delete = $this->machine[$server]["AUTO_DELETE"];
        }

        // Setting exchange and queue information
        $this->exchange = $this->machine[$server]["EXCHANGE"];
        $this->queue = $this->machine[$server]["QUEUE"];
    }

    // Method to connect to the RabbitMQ broker
    private function connectToBroker(){
        foreach ($this->BROKER_HOSTS as $host) {
            try {
                // Establishing connection parameters
                $params = array(
                    'host' => $host,
                    'port' => $this->BROKER_PORT,
                    'login' => $this->USER,
                    'password' => $this->PASSWORD,
                    'vhost' => $this->VHOST
                );

                // Attempting to connect to the broker
                $conn = new AMQPConnection($params);
                if ($conn->connect()) {
                    return $conn;
                }
            } catch (Exception $e) {
                // Logging error if connection fails
                error_log("Connection failed to RabbitMQ broker at $host: " . $e->getMessage());
                sleep(3);
            }
        }
        // Throwing exception if no connection can be made
        throw new Exception("Unable to connect to any RabbitMQ brokers.");
    }

    // Method to process incoming messages
	function process_message($msg)
    {
        // Only process messages with a specific routing key
        if ($msg->getRoutingKey() !== "*")
        {
            return;
        }
        // Acknowledge the message
        $this->conn_queue->ack($msg->getDeliveryTag());
        try
        {
            // Handling response if the message has a reply destination
            if ($msg->getReplyTo())
            {
                $body = $msg->getBody();
                $payload = json_decode($body, true);
                $response = null;
                if (isset($this->callback))
                {
                    // Executing callback function with the message payload
                    $response = call_user_func($this->callback, $payload);
                }

                // Setting up exchange and queue for sending the response
                $conn = $this->connectToBroker();
                $channel = new AMQPChannel($conn);
                $exchange = new AMQPExchange($channel);
                $exchange->setName($this->exchange);
                $exchange->setType($this->exchange_type);

                $conn_queue = new AMQPQueue($channel);
                $conn_queue->setName($msg->getReplyTo());
                $replykey = $this->routing_key . ".response";
                $conn_queue->bind($exchange->getName(), $replykey);
                $exchange->publish(json_encode($response), $replykey, AMQP_NOPARAM, array('correlation_id' => $msg->getCorrelationId()));

                return;
            }
        }
        catch(Exception $e)
        {
            // Handling exceptions during message processing
            echo "error: rabbitMQServer: process_message: exception caught: " . $e;
        }
        // Processing one-way messages without expecting a response
        $body = $msg->getBody();
        $payload = json_decode($body, true);
        if (isset($this->callback))
        {
            // Executing callback function for one-way messages
            call_user_func($this->callback, $payload);
        }
        echo "processed one-way message\n";
    }

    // Method to start processing requests
	function process_requests($callback)
    {
        try {
            // Assigning the callback function for processing messages
            $this->callback = $callback;
            $conn = $this->connectToBroker();

            // Setting up channel and exchange for message processing
            $channel = new AMQPChannel($conn);
            $exchange = new AMQPExchange($channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            // Setting up and binding the queue to the exchange
            $this->conn_queue = new AMQPQueue($channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);

            // Starting the message consumption process
            $this->conn_queue->consume(array($this, 'process_message'));

            // Keeping the channel open to wait for incoming messages
            while (count($channel->callbacks)) {
                $channel->wait();
            }
        } catch (Exception $e) {
            // Handling exceptions if the request processor fails to start
            trigger_error("Failed to start request processor: " . $e->getMessage(), E_USER_ERROR);
        }
    }
}

// Class for managing RabbitMQ Client
class rabbitMQClient
{
    // Private and public variables for RabbitMQ connection configuration
    private $machine = "";
    public $BROKER_HOSTS;
    private $BROKER_PORT;
    private $USER;
    private $PASSWORD;
    private $VHOST;
    private $exchange;
    private $queue;
    private $routing_key = '*';
    private $response_queue = array();
    private $exchange_type = "topic";

    // Constructor to initialize client settings
    function __construct($machine, $server = "rabbitMQ")
    {
        // Retrieve and set client configurations from the provided machine info
        $this->machine = getHostInfo(array($machine));
        $this->BROKER_HOSTS = $this->machine[$server]["BROKER_HOSTS"];
        $this->BROKER_PORT = $this->machine[$server]["BROKER_PORT"];
        $this->USER = $this->machine[$server]["USER"];
        $this->PASSWORD = $this->machine[$server]["PASSWORD"];
        $this->VHOST = $this->machine[$server]["VHOST"];

        // Optional settings for exchange type
        if (isset($this->machine[$server]["EXCHANGE_TYPE"])) {
            $this->exchange_type = $this->machine[$server]["EXCHANGE_TYPE"];
        }

        // Setting exchange and queue information
        $this->exchange = $this->machine[$server]["EXCHANGE"];
        $this->queue = $this->machine[$server]["QUEUE"];
    }

    // Method to connect to the RabbitMQ broker
    private function connectToBroker()
    {
        foreach ($this->BROKER_HOSTS as $host) {
            try {
                // Establishing connection parameters
                $params = array(
                    'host' => $host,
                    'port' => $this->BROKER_PORT,
                    'login' => $this->USER,
                    'password' => $this->PASSWORD,
                    'vhost' => $this->VHOST
                );

                // Attempting to connect to the broker
                $conn = new AMQPConnection($params);
                if ($conn->connect()) {
                    return $conn;
                }
            } catch (Exception $e) {
                // Logging error if connection fails
                error_log("Connection failed to RabbitMQ broker at $host: " . $e->getMessage());
                sleep(3);
            }
        }
        // Throwing exception if no connection can be made
        throw new Exception("Unable to connect to any RabbitMQ brokers.");
    }

    // Method to handle responses from the server
    function process_response($response)
    {
        $uid = $response->getCorrelationId();
        // Check if the response is for a known request
        if (!isset($this->response_queue[$uid])) {
            echo "unknown uid\n";
            return true;
        }

        // Acknowledge the response message
        $this->conn_queue->ack($response->getDeliveryTag());
        $body = $response->getBody();
        $payload = json_decode($body, true);

        // Handle empty response
        if (!isset($payload)) {
            $payload = "[empty response]";
        }

        // Store the response in the response queue
        $this->response_queue[$uid] = $payload;
        return false;
    }

    // Method to send a request to the server
    function send_request($message)
    {
        $uid = uniqid();
        $json_message = json_encode($message);
        try {
            // Establish a connection to the broker
            $conn = $this->connectToBroker();
            $channel = new AMQPChannel($conn);
            $exchange = new AMQPExchange($channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            // Setting up a callback queue for the response
            $callback_queue = new AMQPQueue($channel);
            $callback_queue->setName($this->queue . "_response");
            $callback_queue->setFlags(AMQP_DURABLE);
            $callback_queue->declareQueue();
            $callback_queue->bind($exchange->getName(), $this->routing_key . ".response");

            // Setting up the queue to send the request
            $this->conn_queue = new AMQPQueue($channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);

            // Publishing the request message to the exchange
            $exchange->publish($json_message, $this->routing_key, AMQP_NOPARAM, array('reply_to' => $callback_queue->getName(), 'correlation_id' => $uid));
            // Marking the response as waiting
            $this->response_queue[$uid] = "waiting";
            // Start consuming responses
            $callback_queue->consume(array($this, 'process_response'));

            // Fetching and returning the response
            $response = $this->response_queue[$uid];
            unset($this->response_queue[$uid]);
            return $response;
        } catch (Exception $e) {
            // Handling exceptions if the message sending fails
            die("failed to send message to exchange: " . $e->getMessage() . "\n");
        }
    }

    // Method to publish a message to the server
    function publish($message)
    {
        $json_message = json_encode($message);
        try {
            // Establish a connection to the broker
            $conn = $this->connectToBroker();
            $channel = new AMQPChannel($conn);
            $exchange = new AMQPExchange($channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            // Setting up the queue for publishing the message
            $this->conn_queue = new AMQPQueue($channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);
            // Publishing the message to the exchange
            return $exchange->publish($json_message, $this->routing_key);
        } catch (Exception $e) {
            // Handling exceptions if the message publishing fails
            die("failed to send message to exchange: " . $e->getMessage() . "\n");
        }
    }
}
?>