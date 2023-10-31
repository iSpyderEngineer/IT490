#!/bin/bash

# Check if RabbitMQ server is already running
if systemctl is-active --quiet rabbitmq-server; then
  echo "RabbitMQ server is already running."
  exit 0
fi

# Start RabbitMQ server
sudo service rabbitmq-server start

# Check if RabbitMQ server has started successfully
if systemctl is-active --quiet rabbitmq-server; then
  echo "RabbitMQ server started successfully."
  exit 0
else
  echo "RabbitMQ failed to start."
  exit 1
fi

