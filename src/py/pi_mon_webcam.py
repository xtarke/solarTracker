#!/usr/bin/env python3

import os
import time
import threading,functools,logging
import datetime
import paho.mqtt.client as mqtt
import logging
import argparse

# logFilename = '/home/xtarke/pi_mon-' + time.strftime('%Y%m%d-%H-%M-%S') + '.log'
logFilename = '/home/pi/pi_mon.log'
logging.basicConfig(filename=logFilename,level=logging.DEBUG)

cameraOn = True
timelapseOn = False
temperature = 0

class PeriodicTimer(object):
    def __init__(self, interval):
        self.interval = interval

    def get_temp(self):
        global temperature
        
        temperature = self.measure_temp()
        self.thread = threading.Timer(self.interval, self.get_temp)
        self.thread.start()

    def start(self):
        self.thread = threading.Timer(self.interval, self.get_temp)
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

    temperatureTimer = PeriodicTimer(10)
    temperatureTimer.start()
        
    try:
        while True:
            #os.popen("fswebcam --quiet -D1--jpeg 100 --set brightness=70% --set contrast=90% --set Saturation=1 -r 640x480 my_image.jpg")
            #os.popen("fswebcam --quiet -D1--jpeg 100 --set brightness=70% --set contrast=90% --set Saturation=1 my_image.jpg")
            #os.popen('./v4l2grab.bin -o my_image.jpg')
            os.popen('fswebcam  --quiet --jpeg 100 --resolution 720x480 my_image.jpg')
            
            imageFile = open("./my_image.jpg", "rb")

            try:                
                data = imageFile.read()
                mqttc.publish("camera2/image",data)                   
        
            finally:
                imageFile.close()
           
            mqttc.publish("camera2/temperatura", temperature)

            time.sleep(60)

    except KeyboardInterrupt:
        print('Ending...')
        mqttc.disconnect()
        temperatureTimer.cancel()
        
if __name__ == '__main__':
    main()
