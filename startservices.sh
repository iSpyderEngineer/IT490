#!/usr/bin/sh

ssh -t -p 22 "enter rahi's systemname@192.168.194.225" sudo mysql -u root -p
ssh -t -p 22 it490@192.168.194.26 sudo service rabbitmq-server start
ssh -t -p 22 md523@192.168.194.175 sudo service apache2 start
