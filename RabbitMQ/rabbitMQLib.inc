<?php

require_once('get_host_info.inc');

class rabbitMQServer
{
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

    function __construct($machine, $server = "rabbitMQ")
    {
        $this->machine = getHostInfo(array($machine));
        $this->BROKER_HOSTS = $this->machine[$server]["BROKER_HOSTS"];
        $this->BROKER_PORT = $this->machine[$server]["BROKER_PORT"];
        $this->USER = $this->machine[$server]["USER"];
        $this->PASSWORD = $this->machine[$server]["PASSWORD"];
        $this->VHOST = $this->machine[$server]["VHOST"];

        if (isset($this->machine[$server]["EXCHANGE_TYPE"])) {
            $this->exchange_type = $this->machine[$server]["EXCHANGE_TYPE"];
        }

        if (isset($this->machine[$server]["AUTO_DELETE"])) {
            $this->auto_delete = $this->machine[$server]["AUTO_DELETE"];
        }

        $this->exchange = $this->machine[$server]["EXCHANGE"];
        $this->queue = $this->machine[$server]["QUEUE"];
    }

    private function connectToBroker(){
        foreach ($this->BROKER_HOSTS as $host) {
            try {
                $params = array(
                    'host' => $host,
                    'port' => $this->BROKER_PORT,
                    'login' => $this->USER,
                    'password' => $this->PASSWORD,
                    'vhost' => $this->VHOST
                );

                $conn = new AMQPConnection($params);
                if ($conn->connect()) {
                    return $conn;
                }
            } catch (Exception $e) {
                error_log("Connection failed to RabbitMQ broker at $host: " . $e->getMessage());
                sleep(3);
            }
        }
        throw new Exception("Unable to connect to any RabbitMQ brokers.");
    }

	function process_message($msg)
    {
        if ($msg->getRoutingKey() !== "*")
        {
            return;
        }
        $this->conn_queue->ack($msg->getDeliveryTag());
        try
        {
            if ($msg->getReplyTo())
            {
                $body = $msg->getBody();
                $payload = json_decode($body, true);
                $response = null;
                if (isset($this->callback))
                {
                    $response = call_user_func($this->callback, $payload);
                }

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
            echo "error: rabbitMQServer: process_message: exception caught: " . $e;
        }
        $body = $msg->getBody();
        $payload = json_decode($body, true);
        if (isset($this->callback))
        {
            call_user_func($this->callback, $payload);
        }
        echo "processed one-way message\n";
    }


	function process_requests($callback)
    {
        try {
            $this->callback = $callback;
            $conn = $this->connectToBroker();

            $channel = new AMQPChannel($conn);

            $exchange = new AMQPExchange($channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            $this->conn_queue = new AMQPQueue($channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);

            $this->conn_queue->consume(array($this, 'process_message'));

            while (count($channel->callbacks)) {
                $channel->wait();
            }
        } catch (Exception $e) {
            trigger_error("Failed to start request processor: " . $e->getMessage(), E_USER_ERROR);
        }
    }
}

class rabbitMQClient
{
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

    function __construct($machine, $server = "rabbitMQ")
    {
        $this->machine = getHostInfo(array($machine));
        $this->BROKER_HOSTS = $this->machine[$server]["BROKER_HOSTS"];
        $this->BROKER_PORT = $this->machine[$server]["BROKER_PORT"];
        $this->USER = $this->machine[$server]["USER"];
        $this->PASSWORD = $this->machine[$server]["PASSWORD"];
        $this->VHOST = $this->machine[$server]["VHOST"];

        if (isset($this->machine[$server]["EXCHANGE_TYPE"])) {
            $this->exchange_type = $this->machine[$server]["EXCHANGE_TYPE"];
        }

        if (isset($this->machine[$server]["AUTO_DELETE"])) {
            $this->auto_delete = $this->machine[$server]["AUTO_DELETE"];
        }

        $this->exchange = $this->machine[$server]["EXCHANGE"];
        $this->queue = $this->machine[$server]["QUEUE"];
    }

    private function connectToBroker()
    {
        foreach ($this->BROKER_HOSTS as $host) {
            try {
                $params = array(
                    'host' => $host,
                    'port' => $this->BROKER_PORT,
                    'login' => $this->USER,
                    'password' => $this->PASSWORD,
                    'vhost' => $this->VHOST
                );

                $conn = new AMQPConnection($params);
                if ($conn->connect()) {
                    return $conn;
                }
            } catch (Exception $e) {
                error_log("Connection failed to RabbitMQ broker at $host: " . $e->getMessage());
                sleep(3);
            }
        }
        throw new Exception("Unable to connect to any RabbitMQ brokers.");
    }

    function process_response($response)
    {
        $uid = $response->getCorrelationId();
        if (!isset($this->response_queue[$uid])) {
            echo "unknown uid\n";
            return true;
        }

        $this->conn_queue->ack($response->getDeliveryTag());
        $body = $response->getBody();
        $payload = json_decode($body, true);

        if (!isset($payload)) {
            $payload = "[empty response]";
        }

        $this->response_queue[$uid] = $payload;
        return false;
    }

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

            $callback_queue = new AMQPQueue($channel);
            $callback_queue->setName($this->queue . "_response");
            $callback_queue->setFlags(AMQP_DURABLE);
            $callback_queue->declareQueue();
            $callback_queue->bind($exchange->getName(), $this->routing_key . ".response");

            $this->conn_queue = new AMQPQueue($channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);

            $exchange->publish($json_message, $this->routing_key, AMQP_NOPARAM, array('reply_to' => $callback_queue->getName(), 'correlation_id' => $uid));
            $this->response_queue[$uid] = "waiting";
            $callback_queue->consume(array($this, 'process_response'));

            $response = $this->response_queue[$uid];
            unset($this->response_queue[$uid]);
            return $response;
        } catch (Exception $e) {
            die("failed to send message to exchange: " . $e->getMessage() . "\n");
        }
    }

    function publish($message)
    {
        $json_message = json_encode($message);
        try {
            $conn = $this->connectToBroker();
            $channel = new AMQPChannel($conn);
            $exchange = new AMQPExchange($channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            $this->conn_queue = new AMQPQueue($channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);
            return $exchange->publish($json_message, $this->routing_key);
        } catch (Exception $e) {
            die("failed to send message to exchange: " . $e->getMessage() . "\n");
        }
    }
}
?>