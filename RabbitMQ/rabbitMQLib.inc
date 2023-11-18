<?php

require_once('get_host_info.inc');

class rabbitMQServer
{
    private $machine = "";
    private $BROKER_HOSTS;
    private $BROKER_PORT;
    private $USER;
    private $PASSWORD;
    private $VHOST;
    private $exchange;
    private $queue;
    private $routing_key = '*';
    private $exchange_type = "topic";
    private $auto_delete = false;
    private $currentHostIndex = 0;
    private $conn;
    private $channel;
    private $callback;

    function __construct($machine, $server = "rabbitMQ")
    {
        $this->machine = getHostInfo(array($machine));
        $this->BROKER_HOSTS = explode(",", $this->machine[$server]["BROKER_HOSTS"]);
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

        $this->conn = $this->connectToRabbitMQ();
        $this->channel = new AMQPChannel($this->conn);
    }

    private function getNextHost()
    {
        $host = $this->BROKER_HOSTS[$this->currentHostIndex];
        $this->currentHostIndex = ($this->currentHostIndex + 1) % count($this->BROKER_HOSTS);
        return $host;
    }

    private function connectToRabbitMQ()
    {
        $attempts = count($this->BROKER_HOSTS);
        while ($attempts > 0) {
            $host = $this->getNextHost();
            try {
                error_log("Attempting to connect to host: " . $host);

                $params = [
                    'host' => $host,
                    'port' => $this->BROKER_PORT,
                    'login' => $this->USER,
                    'password' => $this->PASSWORD,
                    'vhost' => $this->VHOST
                ];

                $connection = new AMQPConnection($params);
                $connection->connect();

                if ($connection->isConnected()) {
                    return $connection;
                }
            } catch (Exception $e) {
                error_log("Connection attempt failed: " . $e->getMessage());
            }
            $attempts--;
        }

        throw new Exception("Unable to connect to any RabbitMQ host");
    }



	    function process_message($msg)
    {
        if ($msg->getRoutingKey() !== "*") {
            return;
        }
        $this->conn_queue->ack($msg->getDeliveryTag());

        try {
            if ($msg->getReplyTo()) {
                $body = $msg->getBody();
                $payload = json_decode($body, true);
                $response;

                if (isset($this->callback)) {
                    $response = call_user_func($this->callback, $payload);
                }

                $exchange = new AMQPExchange($this->channel);
                $exchange->setName($this->exchange);
                $exchange->setType($this->exchange_type);

                $replyQueue = new AMQPQueue($this->channel);
                $replyQueue->setName($msg->getReplyTo());
                $replykey = $this->routing_key . ".response";
                $replyQueue->bind($exchange->getName(), $replykey);
                $exchange->publish(json_encode($response), $replykey, AMQP_NOPARAM, array('correlation_id' => $msg->getCorrelationId()));

                return;
            }
        } catch (Exception $e) {
            error_log("Error in process_message: " . $e->getMessage());
        }

        $body = $msg->getBody();
        $payload = json_decode($body, true);
        if (isset($this->callback)) {
            call_user_func($this->callback, $payload);
        }
        echo "Processed one-way message\n";
    }

    function process_requests($callback)
    {
        $this->callback = $callback;

        try {
            $exchange = new AMQPExchange($this->channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            $this->conn_queue = new AMQPQueue($this->channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);

            $this->conn_queue->consume(array($this, 'process_message'));

            while (count($this->channel->callbacks)) {
                $this->channel->wait();
            }
        } catch (Exception $e) {
            error_log("Failed to start request processor: " . $e->getMessage());
        }
    }

    function __destruct()
    {
        if ($this->conn && $this->conn->isConnected()) {
            $this->conn->disconnect();
        }
    }
}


class rabbitMQClient
{
    private $machine = "";
    private $BROKER_HOSTS;
    private $BROKER_PORT;
    private $USER;
    private $PASSWORD;
    private $VHOST;
    private $exchange;
    private $queue;
    private $routing_key = '*';
    private $response_queue = array();
    private $exchange_type = "topic";
    private $currentHostIndex = 0;
    private $conn;
    private $channel;

    function __construct($machine, $server = "rabbitMQ")
    {
        $this->machine = getHostInfo(array($machine));
        $this->BROKER_HOSTS = explode(",", $this->machine[$server]["BROKER_HOSTS"]);
        $this->BROKER_PORT = $this->machine[$server]["BROKER_PORT"];
        $this->USER = $this->machine[$server]["USER"];
        $this->PASSWORD = $this->machine[$server]["PASSWORD"];
        $this->VHOST = $this->machine[$server]["VHOST"];
       
        if (isset($this->machine[$server]["EXCHANGE_TYPE"])) {
            $this->exchange_type = $this->machine[$server]["EXCHANGE_TYPE"];
        }

        $this->exchange = $this->machine[$server]["EXCHANGE"];
        $this->queue = $this->machine[$server]["QUEUE"];

        $this->conn = $this->connectToRabbitMQ();
        $this->channel = new AMQPChannel($this->conn);
    }

    private function getNextHost()
    {
        $host = $this->BROKER_HOSTS[$this->currentHostIndex];
        $this->currentHostIndex = ($this->currentHostIndex + 1) % count($this->BROKER_HOSTS);
        return $host;
    }

    private function connectToRabbitMQ()
    {
        $attempts = count($this->BROKER_HOSTS);
        while ($attempts > 0) {
            $host = $this->getNextHost();
            try {
                error_log("Attempting to connect to host: " . $host);

                $params = [
                    'host' => $host,
                    'port' => $this->BROKER_PORT,
                    'login' => $this->USER,
                    'password' => $this->PASSWORD,
                    'vhost' => $this->VHOST
                ];

                $connection = new AMQPConnection($params);
                $connection->connect();

                if ($connection->isConnected()) {
                    return $connection;
                }
            } catch (Exception $e) {
                error_log("Connection attempt failed: " . $e->getMessage());
            }
            $attempts--;
        }

        throw new Exception("Unable to connect to any RabbitMQ host");
    }

    function process_response($response)
    {
        $uid = $response->getCorrelationId();
        if (!isset($this->response_queue[$uid]))
        {
            echo "unknown uid\n";
            return true;
        }

        $this->conn_queue->ack($response->getDeliveryTag());
        $body = $response->getBody();
        $payload = json_decode($body, true);
        if (!(isset($payload)))
        {
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
            $this->conn = $this->connectToRabbitMQ();
            if (!$this->conn) {
                throw new Exception("Connection failed");
            }
            $this->channel = new AMQPChannel($this->conn);

            $exchange = new AMQPExchange($this->channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            $this->conn_queue = new AMQPQueue($this->channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);

            $exchange->publish($json_message, $this->routing_key, AMQP_NOPARAM, array('reply_to' => $this->queue . "_response", 'correlation_id' => $uid));

            $callback_queue = new AMQPQueue($this->channel);
            $callback_queue->setName($this->queue . "_response");
            $callback_queue->bind($exchange->getName(), $this->routing_key . ".response");

            $this->response_queue[$uid] = "waiting";

            $callback_queue->consume(function($envelope, $queue) use ($uid) {
                if ($envelope->getCorrelationId() == $uid) {
                    $this->response_queue[$uid] = json_decode($envelope->getBody(), true);
                    $queue->ack($envelope->getDeliveryTag());
                    $queue->cancel($envelope->getConsumerTag());
                }
            });

            $response = $this->response_queue[$uid];
            unset($this->response_queue[$uid]);
            return $response;
        } catch (Exception $e) {
            error_log("Failed to send message: " . $e->getMessage());
            return null;
        }
    }

    function publish($message)
    {
        $json_message = json_encode($message);
        try {
            $this->conn = $this->connectToRabbitMQ();
            if (!$this->conn) {
                throw new Exception("Connection failed");
            }
            $this->channel = new AMQPChannel($this->conn);

            $exchange = new AMQPExchange($this->channel);
            $exchange->setName($this->exchange);
            $exchange->setType($this->exchange_type);

            $this->conn_queue = new AMQPQueue($this->channel);
            $this->conn_queue->setName($this->queue);
            $this->conn_queue->bind($exchange->getName(), $this->routing_key);
            
            return $exchange->publish($json_message, $this->routing_key);
        } catch (Exception $e) {
            error_log("Failed to publish message: " . $e->getMessage());
        }
    }


    function __destruct()
    {
        if ($this->conn && $this->conn->isConnected()) {
            $this->conn->disconnect();
        }
    }
}
?>
