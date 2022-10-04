import pika

#CONSUMER CODE

#function to do something when msg is received
#for now, print that mssg was received
#this function will be used as a callback in the basic_consume
def on_message_received(ch, method, properties, body):
	print("Message Received: ", body)

#FIRST ESTABLISH CONNECTION AND CHANNEL WITH RABBIT
#create connection with connection parameters
#host is user and IP of rabbitmq machine (Nick's VM)
#also include password
creds=pika.PlainCredentials(username='nick', password='password123')
connection=pika.BlockingConnection(
        pika.ConnectionParameters(host ='192.168.194.33', credentials=creds))


#create a channel for connection
#connection can have different channels
channel=connection.channel()

#to send a mssg to broker first declare queue and queue name
#doesnt matter if the queue is declared twice
#wherever the code executes first, it will declare the queue
#the other code, the queue declaration is ignored if it is already declared
channel.queue_declare(queue='FirstQueue')

#use basic_consume method to consume off the queue
#indicate queue to consume from and set auto_ack to true to
#automatically acknowledge the mssg

channel.basic_consume(queue='FirstQueue', auto_ack=True, 
	on_message_callback=on_message_received)

print("[*] Starting to Consume Messages... Press CTRL+C to stop")

#lock and start consuming
#everytime it receives a msg it will call the function on_message_received
channel.start_consuming()
