#!/bin/bash

#checks  rabbit MQ
if ping 10.244.1.4 -c 1; then
	#checks if rabbitMQ is on
	sudo ssh vboxuser@10.244.1.4 './start_rabbitmq.sh'

else
	echo "Rabbit MQ Host not online"

fi

#checks back-end 1
if ping 10.244.1.3 -c 1; then
	#check if NGINX is on
	sudo ssh user@10.244.1.3 './back-start.sh'

else
	echo "NGINX Host not online"

fi

#checks back-end 2
if ping 10.244.1.5 -c 1; then
	#check if NGINX is on
	sudo ssh user@10.244.1.5 './back-start.sh'

else
	echo "NGINX HOST not Online"

fi
#checks database
if ping 10.244.1.2 -c 1; then
	#check if  MySQL is on
	sudo ssh jc268@10.244.1.2 'cd IT490 ; ./startDatabase.sh'

else
	echo "MySQL Host not online"
fi
