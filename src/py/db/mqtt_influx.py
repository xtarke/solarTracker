#!/usr/bin/env python3

from influxdb import InfluxDBClient
import paho.mqtt.client as mqtt
import time
import datetime
import logging
import argparse
import subprocess
import os


logFilename = '/home/starke/influxLight-' + time.strftime('%Y%m%d-%H-%M-%S') + '.log'
logging.basicConfig(filename=logFilename,level=logging.DEBUG)
temperature = '0'
az = '0'
ze = '0'


def on_message(client, userdata, message):
    global temperature
    global az
    global ze

    if (message.topic == 'camera1/image'):    
        # print('Image received!')
        newFile = open('/usr/share/grafana/public/img/my_image.jpg', "wb")
        newFile.write(message.payload)
        newFile.close()

        # Make timelapse
        now = datetime.datetime.now().time()
        if ((now > datetime.time(6,0,0)) & (now < datetime.time(20,0,0))):
            folderName = '/home/starke/Pictures/' + time.strftime("%Y%m%d")            
            newFileName = folderName + '/' + time.strftime("%H%M%S") + '.jpg'

            # print(folderName)
            # print(newFileName)
            createFolder(folderName)
            
            newFile = open(newFileName, "wb")
            newFile.write(message.payload)
            newFile.close()        

    if (message.topic == 'camera1/temperatura'):    
        temperature = str(float(message.payload))
        # print(temperature)

    if (message.topic == 'solar/az'):
        az = str(float(message.payload))
        # print(az)

    if (message.topic == 'solar/ze'):
        ze = str(float(message.payload))
        # print(ze)

def on_connect(client, userdata, flags, rc):
    
    print('Connected with result code ' + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("$SYS/#")  

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print('Unexpected disconnection: ' + str(rc))
        logging.warning('Unexpected disconnection: ' + str(rc))

def createFolder(directory):
    try:
        if not os.path.exists(directory):
            os.makedirs(directory)
    except OSError:
        print ('Error: Creating directory. ' + directory)
            
def main():
    json_body = [
        {
            "measurement": "camera_temp",
            "tags": {
                "tracker": "ufsc",
                "region": "florianopolis"
            },
            # "time": "2018-11-18T09:35:00Z",
            "fields": {
                "Float_value": 0.64,
                #"value": 23.0
            }
        }
    ]
    
    parser = argparse.ArgumentParser(description='Simple mqtt to influx gateWay')
    parser.add_argument('broker', help='Broker address')
    parser.add_argument('user', help='Broker user')
    parser.add_argument('password', help='Broker password')
    # parser.add_argument('DBuser', help='InfluxDB user')
    # parser.add_argument('DBpassword', help='InfluxDB password')

    options = parser.parse_args();
        
    print('Connecting to {} with user {}.'.format(options.broker, options.user))
    logging.info('Connecting to {} with user {}.'.format(options.broker, options.user))
        
    mqttc = mqtt.Client()
    mqttc.username_pw_set(options.user,  options.password)
    mqttc.connect(options.broker)
 
    mqttc.loop_start()
    mqttc.on_disconnect = on_disconnect
    mqttc.on_message = on_message
    mqttc.on_connect = on_connect

    mqttc.subscribe("camera1/temperatura")
    logging.info('Subscribing to: ' + 'camera1/temperatura')

    mqttc.subscribe("solar/ze")
    logging.info('Subscribing to: ' + 'solar/ze')

    mqttc.subscribe("solar/az")
    logging.info('Subscribing to: ' + 'solar/az')

    mqttc.subscribe("camera1/image")
    logging.info('Subscribing to: ' + 'camera1/image')

    #client = InfluxDBClient('localhost', 8086, options.DBuser, options.DBpassword)
    client = InfluxDBClient('localhost', 8086)
    client.switch_database('solartracker')

    try:
        while True:
                 
            time.sleep(30)
            json_body[0]['measurement'] = 'camera_temp'
            json_body[0]['fields']['Float_value'] = float(temperature)
            # json_body[0]['time'] = time.strftime('%Y-%m-%dT%H:%M:%S') + str(timezoneinfo) + ':00'            
            # print(json_body)
            client.write_points(json_body)

            now = datetime.datetime.now().time()

            # Record solar position between 6 and 20
            if ((now > datetime.time(6,0,0)) & (now < datetime.time(20,0,0))):
            	json_body[0]['measurement'] = 'azimuth'
            	json_body[0]['fields']['Float_value'] = float(az)
            	client.write_points(json_body)

            	json_body[0]['measurement'] = 'zenith'
            	json_body[0]['fields']['Float_value'] = float(ze)
            	client.write_points(json_body)

            # subprocess.run(['scp', '-P', '9000', 'pi@150.162.29.74:/home/pi/my_image.jpg', '/usr/share/grafana/public/img/my_image.jpg'])
            # subprocess.run(['scp', '-P', ' 9000' ,'pi@150.162.29.74:/home/pi/my_image.jpg', '/usr/share/grafana/public/img/my_image2.jpg'])
               
    except KeyboardInterrupt:
        print('Ending...')
        mqttc.disconnect()
        client.close()
         
            
if __name__ == '__main__':
    main()


