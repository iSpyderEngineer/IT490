import argparse
import socket
import netifaces
import ipaddress
from concurrent.futures import ThreadPoolExecutor, wait
from pprint import pp
import time
import pika
import requests
import sys
import mysql.connector as mysql
import psycopg2

# Source: https:///stackoverflow.com/questions/26174743/making-a-fast-port-scanner
def tcp_connect(ip, port_number):
    TCPsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    TCPsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    TCPsock.settimeout(0.5)
    try:
        TCPsock.connect((ip, port_number))
        return True
    except:
        return False

# Parse command line arguments
parser = argparse.ArgumentParser(
        description="Tests containers found on the network")
parser.add_argument('-i', help="interface to scan", metavar="INTERFACE",
        default='eth0')
parser.add_argument('-t', help="maximum threads to use", metavar="THREADS",
        default=1000)
args = parser.parse_args()
interface = args.i
threads = args.t

pool = ThreadPoolExecutor(threads)

###############################################################################
########################### Host Discovery ####################################
###############################################################################

print(f"Discovering hosts on {interface}...")

hosts = {}
ports = {
    3306 : 'MySQL/MariaDB',
    5432 : 'PostgreSQL',
    5672 : 'RabbitMQ',
}

# create a network of all valid IPs
ipv4if = netifaces.ifaddresses(interface)[netifaces.AF_INET][0]
ourip = ipv4if['addr']
netmask = ipv4if['netmask']
network = ipaddress.ip_network((ourip, netmask), strict=False)

# create threads to check the services of hosts
for addr in network:
    ip = str(addr)
    hosts[ip] = {}
    for port, service in ports.items():
        hosts[ip][service] = pool.submit(tcp_connect, ip, port)

# store the results of each thread
count = 0
for ip in hosts:
    if (count % 10000) == 0:
        print(f"> {count}/{len(hosts)} hosts scanned...")
    for service in hosts[ip]:
        future = hosts[ip][service]
        wait([future])
        hosts[ip][service] = future.result()
    count += 1

# filter out hosts that don't have any services and make a services list
active_hosts = {}
for ip in hosts:
    services = []
    for service in hosts[ip]:
        if hosts[ip][service]:
            services.append(service)
    if len(services) > 0:
        active_hosts[ip] = services

# print the output
pp(active_hosts)

# quit if we have nothing to test
if len(active_hosts) == 0:
    print("No active hosts! Is this running on the correct network?")
    sys.exit(1)

###############################################################################
############################## Service Tests ##################################
###############################################################################

for ip in active_hosts:
    if 'RabbitMQ' in active_hosts[ip]:
        print(f"Performing RabbitMQ tests on {ip}...")

        print("> Checking for ports that shouldn't be open...")
        valid_ports = [4369, 5672, 15672]
        portscan = {}
        for port in range(1, 10000):
            portscan[port] = pool.submit(tcp_connect, ip, port)
        wait(portscan.values())
        for port in portscan:
            if portscan[port].result() and port not in valid_ports:
                print(f"> [FAILED] Port {port} is open")

        print("> Checking connection, queue creation, producing, and consuming...")
        connection = pika.BlockingConnection(pika.ConnectionParameters(ip))
        channel = connection.channel()
        channel.queue_declare(queue='test')
        channel.basic_publish(exchange='', routing_key='test', body='Test Message')
        connection.close()
        time.sleep(5)
        connection = pika.BlockingConnection(pika.ConnectionParameters(ip))
        channel = connection.channel()
        method_frame, header_frame, body = channel.basic_get('test')
        if method_frame:
            channel.basic_ack(method_frame.delivery_tag)
            if body != b'Test Message':
                printf("> [FAILED] Returned message does not match: {body}")
        else:
            print("> [FAILED] No message returned")
        connection.close()

        if tcp_connect(ip, 15672):
            print("> Management plugin is active, checking that default credentials are not used...")
            response = requests.get(f'http://{ip}:15672/api/overview',
                    auth=('guest', 'guest'))
            if response.status_code != 401:
                print("> [FAILED] Default credentials (guest / guest) work")

    if 'MySQL/MariaDB' in active_hosts[ip]:
        print(f"Performing MySQL/MariaDB tests on {ip}...")

        print("> Connecting as 'test' with password 'test' to database 'test'") 
        try:
            db = mysql.connect(
                host     = ip,
                user     = 'test',
                passwd   = 'test',
                database = 'test',
            )
            db.close()
        except mysql.Error as err:
            print(f"> [FAILED] {err}")

    if 'PostgreSQL' in active_hosts[ip]:
        print(f"Performing PostgreSQL tests on {ip}...")

        print("> Connecting as 'test' with password 'test' to database 'test'")
        try:
            conn = psycopg2.connect(
                host     = ip,
                user     = 'test',
                password = 'test',
                dbname   = 'test',
            )
            conn.close()
        except psycopg2.Error as err:
            print(f"> [FAILED] {err}")
