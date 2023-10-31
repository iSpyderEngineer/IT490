#!/bin/bash
#check if instance is already on
sudo aws configure
#check if it is already online
if sudo aws rds wait db-instance-available --db-intance-identifier dbit490-fix --region us-east-1; then
        echo "The RDS server is already online."
        exit 0
fi
#turn on RDS database
sudo aws reds start-bt-instance --db-instance-identifier dbit490-fixed --region us-east-1;
#check if it actually turned on
if sudo aws reds wait db-instance-available --db-instance-identifier dbit490-fixed --region us-east-1; then
        echo "The RDS server is active!"
        exit 1
fi
