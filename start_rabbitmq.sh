#!/bin/bash

# AWS EC2 Instance ID
INSTANCE_ID="i-0f679bf38b3f7a1e1"

# Start the EC2 instance
echo "Starting EC2 instance..."
aws ec2 start-instances --instance-ids "$INSTANCE_ID"

# Wait for the instance to start
echo "Waiting for the instance to start..."
aws ec2 wait instance-status-ok --instance-ids "$INSTANCE_ID"

# Check if RabbitMQ service is running
echo "Checking RabbitMQ service status..."
SSH_COMMAND="ssh -i "vmkey.pem" ubuntu@ec2-3-22-139-189.us-east-2.compute.amazonaws.com"
RABBITMQ_STATUS="$($SSH_COMMAND 'sudo systemctl is-active rabbitmq-server')"

# If RabbitMQ service is not running, start it
if [ "$RABBITMQ_STATUS" != "active" ]; then
    echo "RabbitMQ service is not running. Starting the service..."
    $SSH_COMMAND 'sudo systemctl start rabbitmq-server'
else
    echo "RabbitMQ service is already running."
fi

echo "Script completed."
