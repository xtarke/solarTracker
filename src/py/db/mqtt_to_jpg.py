#!/usr/bin/env python3

# This program receive a image as MQTT payload and write it to a jpg file

import paho.mqtt.client as mqtt
import time
import datetime
import logging
import argparse
import subprocess

logFilename = 'log.log'
logging.basicConfig(filename=logFilename,level=logging.DEBUG)
temperature = '0'
az = '0'
ze = '0'

def on_message(client, userdata, message):
    global temperature
    global az
    global ze
 
    # print("%s %s" % (message.topic, message.payload))    

    if (message.topic == 'camera1/image'):    
        print('Image received!')

        newFile = open('data.jpg', "wb")
        newFile.write(message.payload)
        newFile.close()      

        print('Done!')

def on_connect(client, userdata, flags, rc):
    
    print('Connected with result code ' + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("$SYS/#")  

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print('Unexpected disconnection: ' + str(rc))
        logging.warning('Unexpected disconnection: ' + str(rc))
            
def main():

    parser = argparse.ArgumentParser(description='Simple mqtt to influx gateWay')
    parser.add_argument('broker', help='Broker address')
    parser.add_argument('user', help='Broker user')
    parser.add_argument('password', help='Broker password')
    # parser.add_argument('DBuser', help='InfluxDB user')
    # parser.add_argument('DBpassword', help='InfluxDB password')

    options = parser.parse_args()

    print('Connecting to {} with user {}.'.format(options.broker, options.user))
    logging.info('Connecting to {} with user {}.'.format(options.broker, options.user))
        
    mqttc = mqtt.Client()
    mqttc.username_pw_set(options.user,  options.password)
    mqttc.connect(options.broker)
 
    mqttc.on_disconnect = on_disconnect
    mqttc.on_message = on_message
    mqttc.on_connect = on_connect

    mqttc.subscribe("camera1/image")
    logging.info('Subscribing to: ' + 'camera1/temperatura')

    # Continue the network loop, exit when an error occurs
    rc = 0
    while rc == 0:
        rc = mqttc.loop()
    print("rc: " + str(rc))
        
            
if __name__ == '__main__':
    main()


