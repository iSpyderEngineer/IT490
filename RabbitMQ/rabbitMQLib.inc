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
    private $queue;
    private $routing_key = '*';
    private $exchange_type = "topic";
    private $auto_delete = false;
    private $lastBrokerIndex = 0;
    private $heartbeat = 10;
    private $timeout = 30;
    private $consumerTimeout;
    private $offlineHosts = [];
    private $retryInterval = 60;

    // Initializes server settings based on the provided configuration
    function __construct($machine, $server = "rabbitMQ", $consumerTimeout = 60)
    {
        // Load server configurations
        $this->machine = getHostInfo(array($machine));
        // Assign configuration values
        $this->BROKER_HOSTS = $this->machine[$server]["BROKER_HOSTS"];
        $this->BROKER_PORT = $this->machine[$server]["BROKER_PORT"];
        $this->USER = $this->machine[$server]["USER"];
        $this->PASSWORD = $this->machine[$server]["PASSWORD"];
        $this->VHOST = $this->machine[$server]["VHOST"];
        $this->consumerTimeout = $consumerTimeout;

        // Set optional settings for exchange type and auto delete
        if (isset($this->machine[$server]["EXCHANGE_TYPE"])) {
            $this->exchange_type = $this->machine[$server]["EXCHANGE_TYPE"];
        }
        if (isset($this->machine[$server]["AUTO_DELETE"])) {
            $this->auto_delete = $this->machine[$server]["AUTO_DELETE"];
        }

        // Set exchange and queue information
        $this->exchange = $this->machine[$server]["EXCHANGE"];
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
                    'heartbeat' => $this->heartbeat,
                    'timeout' => $this->timeout
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

    // Processes incoming messages, checking for routing key and handling responses
    function process_message($msg)
    {
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
                $conn = $this->connectToBroker();
                $channel = new AMQPChannel($conn);
                $exchange = new AMQPExchange($channel);
                $exchange->setName($this->exchange);
                $exchange->setType($this->exchange_type);

                $replykey = $this->queue . ".response";
                $exchange->publish(json_encode($response), $replykey, AMQP_NOPARAM, array('correlation_id' => $msg->getCorrelationId()));
            }

            $this->conn_queue->ack($msg->getDeliveryTag());
        } catch (Exception $e) {
            error_log("Error in rabbitMQServer: process_message: " . $e->getMessage());
            $this->conn_queue->nack($msg->getDeliveryTag(), AMQP_REQUEUE);
        }
    }


    // Starts processing requests and continuously listens for messages
	function process_requests($callback) 
    {
        $this->callback = $callback;

        // Infinite loop to keep the server running and processing messages
        while (true) {
            try {
                // Establish a connection and set up message processing environment
                $conn = $this->connectToBroker();
                $channel = new AMQPChannel($conn);
                $exchange = new AMQPExchange($channel);
                $exchange->setName($this->exchange);
                $exchange->setType($this->exchange_type);

                $this->conn_queue = new AMQPQueue($channel);
                $this->conn_queue->setName($this->queue);
                $this->conn_queue->bind($exchange->getName(), $this->routing_key);

                // Inner loop to continuously consume messages
                while (true) {
                    try {
                        // Wait and process incoming messages
                        $this->conn_queue->consume(array($this, 'process_message'));
                        $channel->wait(null, false, $this->consumerTimeout);
                    } catch (AMQPTimeoutException $e) {
                        // Handle timeouts and continue listening
                        error_log("Consumer timeout, continuing to listen: " . $e->getMessage());
                        continue;
                    }
                }
            } catch (AMQPConnectionException $e) {
                // Handle connection exceptions and retry
                error_log("AMQP Connection Exception: " . $e->getMessage());
                sleep(3);
            } catch (Exception $e) {
                // Handle general exceptions and retry
                error_log("General Exception: " . $e->getMessage());
                sleep(3);
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
    private $lastBrokerIndex = 0;
    private $heartbeat = 10;
    private $timeout = 30;
    private $offlineHosts = [];
    private $retryInterval = 60;

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

    // Initializes the response queue for receiving responses from the server
    private function initResponseQueue() 
    {
        $conn = $this->connectToBroker();
        $channel = new AMQPChannel($conn);
        $this->responseQueue = new AMQPQueue($channel);
        $this->responseQueue->setName($this->queue . "_response");
        $this->responseQueue->setFlags(AMQP_DURABLE);
        $this->responseQueue->declareQueue();
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
                    'heartbeat' => $this->heartbeat,
                    'timeout' => $this->timeout
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

        // Acknowledge the response using the responseQueue
        $this->responseQueue->ack($response->getDeliveryTag());

        $body = $response->getBody();
        $payload = json_decode($body, true) ?? "[empty response]";

        $this->response_queue[$uid] = $payload;
        return false;
    }

    // Sends a request to the server and waits for a response
    function send_request($message)
    {
        $uid = uniqid();
        $json_message = json_encode($message);

        try {
            $conn = $this->connectToBroker();
            $channel = new AMQPChannel($conn);
            $exchange = new AMQPExchange($channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            $responseRoutingKey = $this->queue . ".response"; 
            $this->responseQueue->bind($exchange->getName(), $responseRoutingKey);

            $exchange->publish($json_message, $this->routing_key, AMQP_NOPARAM, array(
                'reply_to' => $this->responseQueue->getName(), 
                'correlation_id' => $uid
            ));

            $this->response_queue[$uid] = "waiting";

            $this->responseQueue->consume(array($this, 'process_response'));

            $response = $this->response_queue[$uid];
            unset($this->response_queue[$uid]);
            return $response;
        } catch (AMQPConnectionException $e) {
            error_log("AMQP Connection Exception in send_request: " . $e->getMessage());
        } catch (Exception $e) {
            error_log("General Exception in send_request: " . $e->getMessage());
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