<?php

require_once('get_host_info.inc');

// class for managing RabbitMQ Server communication
class rabbitMQServer
{
    // properties to store RabbitMQ Server configuration
    private $machine = ""; // stores the machine name
    private $BROKER_HOSTS; // stores an array of broker hosts
    private $BROKER_PORT; // stores the broker port
    private $USER; // stores the username for authentication
    private $PASSWORD; // stores the password for authentication
    private $VHOST; // stores the virtual host
    private $exchange; // stores an AMQPExchange object
    private $exchangeName; // stores the name of the exchange
    private $queue; // stores the name of the queue
    private $routing_key = '*'; // stores the routing key for binding
    private $exchange_type = "topic"; // stores the exchange type
    private $auto_delete = false; // sets the exchange to not auto-delete
    private $lastBrokerIndex = 0; // index of the last used broker
    private $heartbeat = 10; // heartbeat interval
    private $offlineHosts = []; // array of offline hosts
    private $retryInterval = 120; // retry interval for offline hosts
    private $conn; // current rabbit connection

    // initializes server settings based on the provided configuration
    function __construct($machine, $server = "rabbitMQ")    
    {
        // load server configurations from class parameter
        $this->machine = getHostInfo(array($machine));
        // assign configuration values from class parameter 
        $this->BROKER_HOSTS = $this->machine[$server]["BROKER_HOSTS"];
        $this->BROKER_PORT = $this->machine[$server]["BROKER_PORT"];
        $this->USER = $this->machine[$server]["USER"];
        $this->PASSWORD = $this->machine[$server]["PASSWORD"];
        $this->VHOST = $this->machine[$server]["VHOST"];

        // set optional settings for exchange type and auto delete
        if (isset($this->machine[$server]["EXCHANGE_TYPE"])) {
            $this->exchange_type = $this->machine[$server]["EXCHANGE_TYPE"];
        }
        if (isset($this->machine[$server]["AUTO_DELETE"])) {
            $this->auto_delete = $this->machine[$server]["AUTO_DELETE"];
        }

        // set exchange and queue information
        $this->exchangeName = $this->machine[$server]["EXCHANGE"];
        $this->queue = $this->machine[$server]["QUEUE"];

        $this->conn = null;
    }

    // initializes the message handler with the necessary configuration
    private function init_request_queue() 
    {
        $this->conn = $this->connect_to_broker();
        $this->validate_connection_and_channel();

        // create a new exchange object and declare it with the class properties
        $this->exchange = new AMQPExchange($this->channel);
        $this->exchange->setName($this->exchangeName); 
        $this->exchange->setType($this->exchange_type);
        
        // ensure the exchange is marked as durable if it's already declared as such on the server
        $this->exchange->setFlags(AMQP_DURABLE);
        $this->exchange->declareExchange();

        // makes an amqp queue object and sets the name and durable flags
        $this->conn_queue = new AMQPQueue($this->channel);
        $this->conn_queue->setName($this->queue);
        $this->conn_queue->setFlags(AMQP_DURABLE);

        // set the queue arguments for dead letter exchange and message TTL
        $queue_arguments = array(
            'x-dead-letter-exchange' => 'dead_letter_exchange',
            'x-dead-letter-routing-key' => 'dead_letter_routing_key',
            'x-message-ttl' => 30000 // TTL for messages in milliseconds
        );
        $this->conn_queue->setArguments($queue_arguments);

        // declare the queue and bind it to the exchange
        $this->conn_queue->declareQueue();
        $this->conn_queue->bind($this->exchangeName, $this->routing_key);
    }

    // connects to a RabbitMQ broker while handling failovers and retries to broker hosts
    private function connect_to_broker() 
    {
        $currentTime = time();
        $brokerCount = count($this->BROKER_HOSTS);

        // loop through broker hosts to find a working connection
        for ($i = 0; $i < $brokerCount; $i++) {
            $index = ($this->lastBrokerIndex + $i) % $brokerCount;
            $host = $this->BROKER_HOSTS[$index];

            // skip any offline hosts based on retry interval
            if (isset($this->offlineHosts[$host]) && $currentTime - $this->offlineHosts[$host] < $this->retryInterval) {
                continue;
            }
            try {
                // attempt to establish a connection
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
                    $this->conn = $conn;
                    // remove host from offline list on successful connection
                    unset($this->offlineHosts[$host]);
                    $this->lastBrokerIndex = ($index + 1) % $brokerCount;
                    return $conn;
                }
            } catch (Exception $e) {
                // add host to offline list on connection failure
                $this->offlineHosts[$host] = $currentTime;
                error_log("Connection failed to RabbitMQ broker at $host: " . $e->getMessage());
            }
        }
        throw new Exception("Unable to connect to any RabbitMQ brokers.");
    }

    // reconnects to RabbitMQ broker
    private function reconnect() 
    {
        // close existing connection if any
        if (isset($this->conn) && $this->conn->isConnected()) {
            $this->conn->disconnect();
        }

        try {
            // re-establish connection
            $this->conn = $this->connect_to_broker();
            $this->channel = new AMQPChannel($this->conn);

            // create a new exchange object and assign it to the class property
            $this->exchange = new AMQPExchange($this->channel);
            $this->exchange->setName($this->exchangeName); 

            // set up the queue
            $this->conn_queue = new AMQPQueue($this->channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($this->exchangeName, $this->routing_key);
        } catch (Exception $e) {
            // log the exception and rethrow to handle the exception further up the call stack
            error_log("Could not reconnect: " . $e->getMessage());
            throw $e;
        }
    }

    // validates the connection and channel
    private function validate_connection_and_channel() 
    {
        // throws an exception if either the connection or channel is invalid
        if (!$this->conn) {
            throw new Exception("Failed to establish a connection to the broker.");
        }
        $this->channel = new AMQPChannel($this->conn);
        if (!$this->channel) {
            throw new Exception("Failed to establish a channel.");
        }
    }

    // closes current connection
    private function close_connection() 
    {
        // check if a connection exists and is connected, then disconnect it
        if ($this->conn && $this->conn->isConnected()) {
            $this->conn->disconnect();
        }
    }

    // processes message from client instance
    function process_message($msg) 
    {
        try {
            $routingKey = $msg->getRoutingKey();
            // check if the message's routing key matches the expected routing key
            if ($routingKey !== $this->routing_key && $routingKey !== $this->queue . ".response") {
                // if false, negatively acknowledge the message and requeue it
                $this->conn_queue->nack($msg->getDeliveryTag(), AMQP_REQUEUE);
                return;
            }

            // extract the message body and decode it as JSON
            $body = $msg->getBody();
            $payload = json_decode($body, true);
            $response = null;

            // if a callback function is set, invoke it with the payload
            if (isset($this->callback)) {
                $response = call_user_func($this->callback, $payload);
            }

            // if the message has a 'replyTo' property, publish the response
            if ($msg->getReplyTo()) {
                $this->publish_response($response, $msg);
            }

            // acknowledge the successful processing of the message
            $this->conn_queue->ack($msg->getDeliveryTag());
        } catch (Exception $e) {
            // log any exceptions that occur during message processing
            error_log("Exception in process_message: " . $e->getMessage());
            // close the connection and rethrow the exception to handle it further up the call stack
            $this->close_connection();
            throw $e;
        }
    }

    // continuously processes incoming messages
    function process_requests($callback) 
    {
        $this->callback = $callback;

        while (true) {
            try {
                // initialize the message handler and consume messages
                $this->init_request_queue();
                $this->consume_messages();
            } catch (Exception $e) {
                // log and handle any errors that occur during message processing
                error_log("Error in process_requests: " . $e->getMessage());
                // close the connection
                $this->close_connection();
                // sleep before retrying after an error
                sleep(3);
            }
        }
    }

    // publishes a response message
    private function publish_response($response, $msg) 
    {
        // check if the channel is available for publishing, throw an exception if false
        if (!$this->channel) {
            throw new Exception("Channel is not available for publishing the response.");
        }
        // if the exchange is not initialized, create and initialize it
        if (!$this->exchange) {
            $this->exchange = new AMQPExchange($this->channel);
            $this->exchange->setName($this->exchangeName);
        }
        // set the exchange type and publish the response message
        $this->exchange->setType($this->exchange_type);

        // set the reply key based on the queue and add the response message to the exchange
        $replykey = $this->queue . ".response";
        $this->exchange->publish(json_encode($response), $replykey, AMQP_NOPARAM, array(
            'correlation_id' => $msg->getCorrelationId()
        ));
    }

    // continuously consumes messages
    private function consume_messages() 
    {
        // sets the prefetch count to the channel
        $prefetchCount = 1;
        $this->channel->qos(null, $prefetchCount);

        // while loop to consume any messages
        while (true) {
            $this->conn_queue->consume(array($this, 'process_message'));
            $this->channel->wait();
        }
    }

    // destructor to close current connection when the class object is destroyed
    function __destruct() {
        $this->close_connection();
    }
}

// class for managing RabbitMQ Client communication
class rabbitMQClient
{
    // properties to store RabbitMQ Client configuration
    private $machine = ""; // stores the machine name
    private $BROKER_HOSTS; // stores an array of broker hosts
    private $BROKER_PORT; // stores the broker port
    private $USER; // stores the username for authentication
    private $PASSWORD; // stores the password for authentication
    private $VHOST; // stores the virtual host
    private $exchange; // stores an AMQPExchange object
    private $queue; // stores the name of the queue
    private $routing_key = '*'; // stores the routing key for binding
    private $exchange_type = "topic"; // stores the exchange type
    private $responseQueue = null; // stores a name for responseQueue
    private $lastBrokerIndex = 0; // stores index of the last used broker
    private $heartbeat = 10; // stores heartbeat interval
    private $offlineHosts = []; // stores a list of offline hosts
    private $retryInterval = 600; // stores retry interval for offline hosts
    private $conn; // stores current rabbit connection

    // constructor to initialize client settings based on the provided configuration
    function __construct($machine, $server = "rabbitMQ")
    {
        // load client configurations from class parameter
        $this->machine = getHostInfo(array($machine));
        $this->BROKER_HOSTS = $this->machine[$server]["BROKER_HOSTS"];
        $this->BROKER_PORT = $this->machine[$server]["BROKER_PORT"];
        $this->USER = $this->machine[$server]["USER"];
        $this->PASSWORD = $this->machine[$server]["PASSWORD"];
        $this->VHOST = $this->machine[$server]["VHOST"];

        // optional settings for exchange type
        if (isset($this->machine[$server]["EXCHANGE_TYPE"])) {
            $this->exchange_type = $this->machine[$server]["EXCHANGE_TYPE"];
        }

        // set exchange and queue information
        $this->exchange = $this->machine[$server]["EXCHANGE"];
        $this->queue = $this->machine[$server]["QUEUE"];

        // initialize the response queue
        $this->init_response_queue();

        $this->conn = null;
    }

    // initializes the response queue
    private function init_response_queue()
    {
        // connect to the RabbitMQ broker
        $conn = $this->connect_to_broker();
        $channel = new AMQPChannel($conn);

        // create an exchange instance and set its name
        $this->exchangeInstance = new AMQPExchange($channel);
        $this->exchangeInstance->setName($this->exchange);

        // create a response queue instance
        $this->responseQueue = new AMQPQueue($channel);
        $this->responseQueue->setName($this->queue . "_response");
        $this->responseQueue->setFlags(AMQP_DURABLE);

        // configure queue arguments regarding dead letter exchange and message TTL
        $queue_arguments = array(
            'x-dead-letter-exchange' => 'dead_letter_exchange',
            'x-dead-letter-routing-key' => 'dead_letter_routing_key',
            'x-message-ttl' => 30000 // 30 seconds expressed in milliseconds
        );

        $this->responseQueue->setArguments($queue_arguments);
        $this->responseQueue->declareQueue();
        $this->responseQueue->bind($this->exchange, $this->routing_key . ".response");
    }

    // connects to a RabbitMQ broker
    private function connect_to_broker() 
    {
        $currentTime = time();
        $brokerCount = count($this->BROKER_HOSTS);

        // loop through broker hosts to establish a connection
        for ($i = 0; $i < $brokerCount; $i++) {
            $index = ($this->lastBrokerIndex + $i) % $brokerCount;
            $host = $this->BROKER_HOSTS[$index];

            // skip offline hosts if the retry interval has not passed
            if (isset($this->offlineHosts[$host]) && $currentTime - $this->offlineHosts[$host] < $this->retryInterval) {
                continue;
            }

            try {
                // attempt to establish a connection with RabbitMQ broker
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
                    // when the connection is successful, remove host from offline list if listed
                    $this->conn = $conn;
                    unset($this->offlineHosts[$host]);
                    $this->lastBrokerIndex = ($index + 1) % $brokerCount;
                    return $conn;
                }
            } catch (Exception $e) {
                // when the connection fails, mark host as offline and log the error
                $this->offlineHosts[$host] = $currentTime;
                error_log("Connection failed to RabbitMQ broker at $host: " . $e->getMessage());
            }
        }
        // if no connection is established, throw an exception
        throw new Exception("Unable to connect to any RabbitMQ brokers.");
    }

    // closes the connection
    private function close_connection() 
    {
        if ($this->conn && $this->conn->isConnected()) {
            $this->conn->disconnect();
        }
    }

    // processes responses received from the server
    function process_response($response) 
    {
        // extract the correlation ID from the response
        $uid = $response->getCorrelationId();

        // check if the correlation ID is known (matches a sent request)
        if (!isset($this->response_queue[$uid])) {
            // log an error for an unknown correlation ID
            error_log("Unknown UID: $uid");
            return true;
        }

        // acknowledge the response from the server
        $this->responseQueue->ack($response->getDeliveryTag());

        // decode the response body as JSON
        $body = $response->getBody();
        $payload = json_decode($body, true);

        // store the response in the response queue
        $this->response_queue[$uid] = $payload ?? "[empty response]";

        // indicate that the response processing is not complete
        return false;
    }

    function send_request($message) 
    {
        // generate a unique identifier for the request
        $uid = uniqid();
        $json_message = json_encode($message);

        try {
            // initialize the response queue for handling the response
            $this->init_response_queue();
            // publish the message to the server with a correlation ID
            $this->publish_message($json_message, $uid);
            // wait for and retrieve the response from the server
            return $this->wait_for_response($uid);
        } catch (Exception $e) {
            // log an error, close the connection, and rethrow the exception
            error_log("Error in send_request: " . $e->getMessage());
            $this->close_connection();
            throw $e;
        }
    }

    // publishes a message to the server
    private function publish_message($json_message, $uid) 
    {
        // publish the JSON message to the server's exchange with relevant metadata
        $this->exchangeInstance->publish($json_message, $this->routing_key, AMQP_NOPARAM, array(
            'reply_to' => $this->responseQueue->getName(),
            'correlation_id' => $uid
        ));
    }

    // waits for a response from the server
    private function wait_for_response($uid) 
    {
        // mark the response as "waiting" in the response queue
        $this->response_queue[$uid] = "waiting";

        // continuously consume responses and process them until not "waiting"
        while (isset($this->response_queue[$uid]) && $this->response_queue[$uid] == "waiting") {
            $this->responseQueue->consume(array($this, 'process_response'));
        }

        // retrieve and return the response from the response queue
        $response = $this->response_queue[$uid] ?? null;
        unset($this->response_queue[$uid]);
        return $response;
    }

    // publishes a message to the server
    function publish($message)
    {
        $json_message = json_encode($message);
        try {
            // connect to the broker and set up the exchange and queue
            $conn = $this->connect_to_broker();
            $channel = new AMQPChannel($conn);
            $exchange = new AMQPExchange($channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            $this->conn_queue = new AMQPQueue($channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);

            // publish the message to the server's exchange
            return $exchange->publish($json_message, $this->routing_key);
        } catch (AMQPConnectionException $e) {
            // log an error for AMQP connection exception
            error_log("AMQP Connection Exception in publish: " . $e->getMessage());
        } catch (Exception $e) {
            // log a general exception error
            error_log("General Exception in publish: " . $e->getMessage());
        }
    }

    // destructor function to close the connection when the class object is destroyed
    function __destruct() 
    {
        // close the connection when object is destroyed
        if ($this->conn && $this->conn->isConnected()) {
            $this->conn->disconnect();
        }
    }
}
?>