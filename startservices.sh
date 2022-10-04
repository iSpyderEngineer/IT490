#!/usr/bin/sh

ssh -t -p 22 rahi@192.168.194.201 sudo mysql -u root -p
ssh -t -p 22 it490@192.168.194.33 sudo service rabbitmq-server start
ssh -t -p 22 md523@192.168.194.253 sudo service apache2 start
