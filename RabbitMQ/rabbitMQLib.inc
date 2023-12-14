<?php

require_once('get_host_info.inc');

// Class for managing RabbitMQ Server communication
class rabbitMQServer
{
    // Properties to store RabbitMQ server configuration
    private $machine = "";
    public $BROKER_HOSTS;
    private $BROKER_PORT;
    private $USER;
    private $PASSWORD;
    private $VHOST;
    private $exchange;
    private $exchangeName;
    private $queue;
    private $routing_key = '*';
    private $exchange_type = "topic";
    private $auto_delete = false;
    private $lastBrokerIndex = 0; // Index of the last used broker
    private $heartbeat = 10; // Heartbeat interval
    private $offlineHosts = []; // List of offline hosts
    private $retryInterval = 60; // Retry interval for offline hosts
    private $response_queue = []; // Queue for storing responses

    // Initializes server settings based on the provided configuration
    function __construct($machine, $server = "rabbitMQ")    
    {
        // Load server configurations
        $this->machine = getHostInfo(array($machine));
        // Assign configuration values
        $this->BROKER_HOSTS = $this->machine[$server]["BROKER_HOSTS"];
        $this->BROKER_PORT = $this->machine[$server]["BROKER_PORT"];
        $this->USER = $this->machine[$server]["USER"];
        $this->PASSWORD = $this->machine[$server]["PASSWORD"];
        $this->VHOST = $this->machine[$server]["VHOST"];

        // Set optional settings for exchange type and auto delete
        if (isset($this->machine[$server]["EXCHANGE_TYPE"])) {
            $this->exchange_type = $this->machine[$server]["EXCHANGE_TYPE"];
        }
        if (isset($this->machine[$server]["AUTO_DELETE"])) {
            $this->auto_delete = $this->machine[$server]["AUTO_DELETE"];
        }

        // Set exchange and queue information
        $this->exchangeName = $this->machine[$server]["EXCHANGE"];
        $this->queue = $this->machine[$server]["QUEUE"];
    }

    // Connects to a RabbitMQ broker, handling failovers and retries
    private function connectToBroker() 
    {
        $currentTime = time();
        $brokerCount = count($this->BROKER_HOSTS);

        // Loop through broker hosts to find a viable connection
        for ($i = 0; $i < $brokerCount; $i++) {
            $index = ($this->lastBrokerIndex + $i) % $brokerCount;
            $host = $this->BROKER_HOSTS[$index];

            // Skip offline hosts based on the retry interval
            if (isset($this->offlineHosts[$host]) && $currentTime - $this->offlineHosts[$host] < $this->retryInterval) {
                continue;
            }
            try {
                // Attempt to establish a connection
                $params = array(
                    'host' => $host,
                    'port' => $this->BROKER_PORT,
                    'login' => $this->USER,
                    'password' => $this->PASSWORD,
                    'vhost' => $this->VHOST,
                    'heartbeat' => $this->heartbeat
                );
                $conn = new AMQPConnection($params);
                if ($conn->connect()) {
                    // Remove host from offline list on successful connection
                    unset($this->offlineHosts[$host]);
                    $this->lastBrokerIndex = ($index + 1) % $brokerCount;
                    return $conn;
                }
            } catch (Exception $e) {
                // Mark host as offline on connection failure
                $this->offlineHosts[$host] = $currentTime;
                error_log("Connection failed to RabbitMQ broker at $host: " . $e->getMessage());
            }
        }
        throw new Exception("Unable to connect to any RabbitMQ brokers.");
    }

    // Reconnects to RabbitMQ broker
    private function reconnect() {
        // Close existing connection if any
        if (isset($this->conn) && $this->conn->isConnected()) {
            $this->conn->disconnect();
        }

        try {
            // Re-establish connection
            $this->conn = $this->connectToBroker();
            $this->channel = new AMQPChannel($this->conn);

            // Create a new exchange object and assign it to the class property
            $this->exchange = new AMQPExchange($this->channel);
            $this->exchange->setName($this->exchangeName); 

            // Set up the queue
            $this->conn_queue = new AMQPQueue($this->channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($this->exchangeName, $this->routing_key);
        } catch (Exception $e) {
            // Log the exception and handle it
            error_log("Could not reconnect: " . $e->getMessage());
            throw $e;
        }
    }

    // Processes incoming messages, checking for routing key and handling responses
    function process_message($msg) {
        try {
            $routingKey = $msg->getRoutingKey();
            if ($routingKey !== $this->routing_key && $routingKey !== $this->queue . ".response") {
                $this->conn_queue->nack($msg->getDeliveryTag(), AMQP_NOPARAM);
                return;
            }

            $body = $msg->getBody();
            $payload = json_decode($body, true);
            $response = null;

            if (isset($this->callback)) {
                $response = call_user_func($this->callback, $payload);
            }

            if ($msg->getReplyTo()) {
                if (!$this->channel) {
                    throw new Exception("Channel is not available for publishing the response.");
                }
                if (!$this->exchange) {
                    // Initialize the exchange if not already done
                    $this->exchange = new AMQPExchange($this->channel);
                    $this->exchange->setName($this->exchangeName);
                }
                $this->exchange->setType($this->exchange_type);

                $replykey = $this->queue . ".response";
                $this->exchange->publish(json_encode($response), $replykey, AMQP_NOPARAM, array(
                    'correlation_id' => $msg->getCorrelationId()
                ));
            }

            // Acknowledge the message if no exception occurs
            $this->conn_queue->ack($msg->getDeliveryTag());
        } catch (Exception $e) {
            error_log("Exception in process_message: " . $e->getMessage());
            $this->reconnect();
            $this->conn_queue->nack($msg->getDeliveryTag(), AMQP_REQUEUE);
        }
    }

    // Starts processing requests and continuously listens for messages
    function process_requests($callback) {
        $this->callback = $callback;

        while (true) {
            try {
                // Establish a connection and set up message processing environment
                $this->conn = $this->connectToBroker();
                if (!$this->conn) {
                    throw new Exception("Failed to establish a connection to the broker.");
                }
                $this->channel = new AMQPChannel($this->conn);
                if (!$this->channel) {
                    throw new Exception("Failed to establish a channel.");
                }

                $exchange = new AMQPExchange($this->channel);
                $exchange->setName($this->exchangeName);
                $exchange->setType($this->exchange_type);

                $this->conn_queue = new AMQPQueue($this->channel);
                $this->conn_queue->setName($this->queue);
                $this->conn_queue->bind($exchange->getName(), $this->routing_key);

                // Inner loop to continuously consume messages
                while (true) {
                    $this->conn_queue->consume(array($this, 'process_message'));
                    $this->channel->wait();
                }
            } catch (Exception $e) {
                error_log("Error in process_requests: " . $e->getMessage());
                // Decide on error handling strategy here (e.g., retry, sleep, exit)
                sleep(3); // Example: Wait for 3 seconds before retrying
            }
        }
    }
}

// Class for managing RabbitMQ Client operations
class rabbitMQClient
{
    // Properties to store RabbitMQ client configuration
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
    private $responseQueue = null;
    private $lastBrokerIndex = 0; // Index of the last used broker
    private $heartbeat = 10; // Heartbeat interval
    private $offlineHosts = []; // List of offline hosts
    private $retryInterval = 60; // Retry interval for offline hosts

    // Constructor to initialize client settings based on the provided configuration
    function __construct($machine, $server = "rabbitMQ")
    {
        // Load and set client configurations from the specified machine
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

        // Set exchange and queue information
        $this->exchange = $this->machine[$server]["EXCHANGE"];
        $this->queue = $this->machine[$server]["QUEUE"];

        // Initialize the response queue
        $this->initResponseQueue();
    }

    private function initResponseQueue() {
        $conn = $this->connectToBroker();
        $channel = new AMQPChannel($conn);

        // Initialize the exchange instance
        $this->exchangeInstance = new AMQPExchange($channel);
        $this->exchangeInstance->setName($this->exchange); // This should be the exchange name

        // Initialize the response queue
        $this->responseQueue = new AMQPQueue($channel);
        $this->responseQueue->setName($this->queue . "_response");
        $this->responseQueue->setFlags(AMQP_DURABLE);
        $this->responseQueue->declareQueue();
        $this->responseQueue->bind($this->exchange, $this->routing_key . ".response"); // Use the exchange name here
    }

    // Connects to a RabbitMQ broker, handling failovers and retries
    private function connectToBroker() 
    {
        $currentTime = time();
        $brokerCount = count($this->BROKER_HOSTS);

        // Loop through broker hosts to find a viable connection
        for ($i = 0; $i < $brokerCount; $i++) {
            $index = ($this->lastBrokerIndex + $i) % $brokerCount;
            $host = $this->BROKER_HOSTS[$index];

            // Skip offline hosts if the retry interval has not passed
            if (isset($this->offlineHosts[$host]) && $currentTime - $this->offlineHosts[$host] < $this->retryInterval) {
                continue;
            }

            try {
                // Attempt to establish a connection
                $params = array(
                    'host' => $host,
                    'port' => $this->BROKER_PORT,
                    'login' => $this->USER,
                    'password' => $this->PASSWORD,
                    'vhost' => $this->VHOST,
                    'heartbeat' => $this->heartbeat
                );

                $conn = new AMQPConnection($params);
                if ($conn->connect()) {
                    // Connection successful, remove from offline hosts if it's listed
                    unset($this->offlineHosts[$host]);
                    $this->lastBrokerIndex = ($index + 1) % $brokerCount;
                    return $conn;
                }
            } catch (Exception $e) {
                // Connection failed, mark host as offline
                $this->offlineHosts[$host] = $currentTime;
                error_log("Connection failed to RabbitMQ broker at $host: " . $e->getMessage());
            }
        }
        throw new Exception("Unable to connect to any RabbitMQ brokers.");
    }

    // Processes responses received from the server
    function process_response($response) 
    {
        $uid = $response->getCorrelationId();
        if (!isset($this->response_queue[$uid])) {
            error_log("Unknown UID: $uid");
            return true;
        }

        // Acknowledge the response message
        $this->responseQueue->ack($response->getDeliveryTag());

        $body = $response->getBody();
        $payload = json_decode($body, true);
        $this->response_queue[$uid] = $payload ?? "[empty response]";
        return false;
    }


    // Sends a request to the server and waits for a response
    function send_request($message) {
        $uid = uniqid();
        $json_message = json_encode($message);

        try {
            // Ensure the response queue is initialized
            $this->initResponseQueue();

            // Now, we use the exchange instance to publish the message
            $this->exchangeInstance->publish($json_message, $this->routing_key, AMQP_NOPARAM, array(
                'reply_to' => $this->responseQueue->getName(),
                'correlation_id' => $uid
            ));

            // Initialize the response queue to an empty state
            $this->response_queue[$uid] = "waiting";

            // Consume messages from the response queue
            while (isset($this->response_queue[$uid]) && $this->response_queue[$uid] == "waiting") {
                $this->responseQueue->consume(array($this, 'process_response'));
            }

            // Retrieve the response
            $response = $this->response_queue[$uid] ?? null;
            // Clean up the queue
            unset($this->response_queue[$uid]);

            return $response; // Return the response received from the server

        } catch (Exception $e) {
            // Log the exception and return null or false to indicate failure
            error_log("Error in send_request: " . $e->getMessage());
            return null;
        }
    }

    // Publishes a message to the server without waiting for a response
    function publish($message)
    {
        $json_message = json_encode($message);
        try {
            // Establish a connection for message publishing
            $conn = $this->connectToBroker();
            $channel = new AMQPChannel($conn);
            $exchange = new AMQPExchange($channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            // Set up the queue for message publishing
            $this->conn_queue = new AMQPQueue($channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);
            // Publish the message to the exchange
            return $exchange->publish($json_message, $this->routing_key);
        } catch (AMQPConnectionException $e) {
            error_log("AMQP Connection Exception in publish: " . $e->getMessage());
        } catch (Exception $e) {
            error_log("General Exception in publish: " . $e->getMessage());
        }
    }
}
?>