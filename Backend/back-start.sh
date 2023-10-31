#!/bin/bash

#checks  rabbit MQ
if ping 10.244.1.4 -c 1; then
	#checks if rabbitMQ is on
	ssh -t vboxuser@10.244.1.4 'sudo ./start_rabbitmq.sh'

else
	echo "Rabbit MQ Host not online"

fi

#checks back-end 1
if ping 10.244.1.3 -c 1; then
	#check if NGINX is on
	ssh -t user@10.244.1.3 'sudo ./back-start.sh'

else
	echo "NGINX Host not online"

fi

#checks back-end 2
if ping 10.244.1.5 -c 1; then
	#check if NGINX is on
	ssh -t user@10.244.1.5 './sudo back-start.sh'

else
	echo "NGINX HOST not Online"

fi
#checks database
if ping 10.244.1.2 -c 1; then
	#check if  MySQL is on
	ssh -t jc268@10.244.1.2 'cd IT490 ; sudo ./startDatabase.sh'

else
	echo "MySQL Host not online"
fi
#checks front end
if ping 10.244.1.6 -c 1; then
	#check if NGINX Front edn is on
 	shh -t sea@10.244.1.6 'sudo ./startwebsite.sh'
else
	echo "NGINX Host Front End is Offline"
fi
