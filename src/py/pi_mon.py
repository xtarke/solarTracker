#!/usr/bin/env python3

import os
import time
import threading,functools,logging
import datetime
from picamera import PiCamera
import paho.mqtt.client as mqtt

cameraOn = True
timelapseOn = False

class PeriodicTimer(object):
    def __init__(self, interval, mqttClient):
        self.interval = interval
        self.mqtt = mqttClient

    def foo(self):
        temperature = self.measure_temp()
        self.mqtt.publish("camera/temperatura", temperature)

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
        print("Unexpected disconnection.")


def createFolder(directory):
    try:
        if not os.path.exists(directory):
            os.makedirs(directory)
    except OSError:
        print ('Error: Creating directory. ' + directory)

def on_message_print(client, userdata, message):
    global cameraOn
    global timelapseOn

    print("%s %s" % (message.topic, message.payload))    

    if (message.topic == 'camera/on'):    
        if (message.payload == b'1'):        
            cameraOn = True        
        else:
            cameraOn = False

    if (message.topic == 'camera/lapse'):
        if (message.payload == b'1'):        
            cameraOn = True
            timelapseOn = True        
        else:
            timelapseOn = False
        

def main():

    mqttc = mqtt.Client()

    mqttc.connect("192.168.0.10")
    mqttc.loop_start()
    mqttc.on_disconnect = on_disconnect
    mqttc.on_message = on_message_print

    mqttc.subscribe("camera/on")
    mqttc.subscribe("camera/lapse")

    temperatureTimer = PeriodicTimer(1, mqttc)
    temperatureTimer.start()

    camera = PiCamera()
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
                    camera.start_preview()
                    time.sleep(2)
                
                my_file = open('my_image.jpg', 'wb')                
                camera.capture(my_file)           

                my_file.close()
                imageFile = open("my_image.jpg", "rb")

                try:                
                    data = imageFile.read()
                    mqttc.publish("camera/image",data)                    
            
                finally:
                    imageFile.close()

                if (sleepCounter == 20):
                    if (timelapseOn == True):                
                        lapseFilename = './timelapse/img_' + time.strftime("%Y%m%d-%H%M%S") + '.jpg'
                        os.rename('./my_image.jpg', lapseFilename)
                        print(lapseFilename)
                        sleepCounter = 0
                    
            else:
                camera.close()
            

            time.sleep(3)
            sleepCounter = sleepCounter + 1

    except KeyboardInterrupt:
        print('Ending...')
        mqttc.disconnect()

        camera.close()

        temperatureTimer.cancel()
        
        

if __name__ == '__main__':
    main()
