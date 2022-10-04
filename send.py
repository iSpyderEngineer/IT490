import pika

#PRODUCER CODE

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
channel.queue_declare(queue='FirstQueue')

#define message
message="Hello World"

#everything has to go through exhange cannot publish mssg directly to queue
#default exchange is blank ''. Routing key is same as queue name
#body is the message
channel.basic_publish(exchange='', routing_key='FirstQueue', body=message)

#print to show that message is sent
print("Message Sent: ", message)
#close the connection
connection.close()
