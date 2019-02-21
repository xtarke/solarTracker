#!/usr/bin/env python3

import os
import time
import threading,functools,logging
import datetime
from picamera import PiCamera
import paho.mqtt.client as mqtt
import logging
import argparse

logFilename = '/home/pi/pi_mon-' + time.strftime('%Y%m%d-%H-%M-%S') + '.log'
logging.basicConfig(filename=logFilename,level=logging.DEBUG)

cameraOn = True
timelapseOn = False
temperature = 0

class PeriodicTimer(object):
    def __init__(self, interval, mqttClient):
        self.interval = interval
        self.mqtt = mqttClient

    def foo(self):
        global temperature
        
        temperature = self.measure_temp()
        self.thread = threading.Timer(self.interval, self.foo)
        self.thread.start()

    def start(self):
        self.thread = threading.Timer(self.interval, self.foo)
        self.thread.start()

    def measure_temp(self):
        temp = os.popen("vcgencmd measure_temp").readline()
        return (temp.replace("temp=","")).replace("'C","")

    def cancel(self):
        self.thread.cancel()
        
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

def on_message_print(client, userdata, message):
    global cameraOn
    global timelapseOn
    
    logging.info('Connecting to {} with user {}.'.format(message.topic, message.payload))
    print("%s %s" % (message.topic, message.payload))    

    if (message.topic == 'camera2/on'):    
        if (message.payload == b'1'):        
            cameraOn = True        
        else:
            cameraOn = False

    if (message.topic == 'camera2/lapse'):
        if (message.payload == b'1'):        
            cameraOn = True
            timelapseOn = True        
        else:
            timelapseOn = False
        

def main():
    parser = argparse.ArgumentParser(description='Simple mqtt PiCamera Monitor')
    parser.add_argument('broker', help='Broker address')
    parser.add_argument('user', help='Broker user')
    parser.add_argument('password', help='Broker password')
    
    options = parser.parse_args();
    
    print('Connecting to {} with user {}.'.format(options.broker, options.user))
    logging.info('Connecting to {} with user {}.'.format(options.broker, options.user))
        
    mqttc = mqtt.Client()
    mqttc.username_pw_set(options.user,  options.password)
    mqttc.connect(options.broker)
    mqttc.loop_start()
    mqttc.on_disconnect = on_disconnect
    mqttc.on_message = on_message_print
    # mqttc.user_data_set(userdata)

    mqttc.subscribe("camera2/on")
    mqttc.subscribe("camera2/lapse")

    temperatureTimer = PeriodicTimer(1, mqttc)
    temperatureTimer.start()

    camera = PiCamera()
    camera.rotation = 0
    camera.iso = 500
    camera.start_preview()

    time.sleep(2)
    #stream = BytesIO()
    createFolder('timelapse')

    sleepCounter = 0
    
    try:
        while True:

            if (cameraOn == True):

                if (camera.closed == True):
                    camera = PiCamera()
                    camera.rotation = 0
                    camera.iso = 500
                    camera.start_preview()
                    time.sleep(2)
                
                camera.annotate_text = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                my_file = open('/home/pi/my_image.jpg', 'wb')                
                camera.capture(my_file)           

                # my_file.close()
                # imageFile = open("/home/pi/my_image.jpg", "rb")

                # print('oi')

                #try:                
                #    data = imageFile.read()
                #    mqttc.publish("camera2/image",data)
                    # print('caca')                 
            
                #finally:
                #    imageFile.close()

                if (sleepCounter > 10):
                    print('camera');
                    if (timelapseOn == True):                
                        lapseFilename = '/home/pi/timelapse/img_' + time.strftime("%Y%m%d-%H%M%S") + '.jpg'
                        os.rename('/home/pi/my_image.jpg', lapseFilename)
                        print(lapseFilename)
                        sleepCounter = 0
                    
            else:
                camera.close()
                    
            # print('hello')
            mqttc.publish("camera2/temperatura", temperature)

            time.sleep(3)
            sleepCounter = sleepCounter + 1

    except KeyboardInterrupt:
        print('Ending...')
        mqttc.disconnect()

        camera.close()

        temperatureTimer.cancel()
        
        

if __name__ == '__main__':
    main()
