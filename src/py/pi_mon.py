import os
import time
import threading,functools
import datetime
from picamera import PiCamera
import paho.mqtt.client as mqtt


cameraOn = False


class PeriodicTimer(object):
    def __init__(self, interval, callback):
        self.interval = interval

        @functools.wraps(callback)
        def wrapper(*args, **kwargs):
            result = callback(*args, **kwargs)
            if result:
                self.thread = threading.Timer(self.interval,
                                              self.callback)
                self.thread.start()

        self.callback = wrapper

    def start(self):
        self.thread = threading.Timer(self.interval, self.callback)
        self.thread.start()

    def cancel(self):
        self.thread.cancel()


        
def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection.")

def measure_temp():
    temp = os.popen("vcgencmd measure_temp").readline()
    return (temp.replace("temp=",""))

def on_message_print(client, userdata, message):
    global cameraOn

    print("%s %s" % (message.topic, message.payload))
    print("renan")
    
    if (message.payload == b'1'):
        print("on");
        #camera.start_preview()
        cameraOn = True        
    else:
        print("off")
        cameraOn = False
        #camera.stop_preview()


def foo():
    print('Doing some work...')
    return True

def main():

    mqttc = mqtt.Client()

    mqttc.connect("192.168.25.51")
    mqttc.loop_start()
    mqttc.on_disconnect = on_disconnect
    mqttc.on_message = on_message_print

    mqttc.subscribe("camera/on")

    timer = PeriodicTimer(1, foo)
    timer.start()

    try:
        while True:
            temperature = measure_temp()
            mqttc.publish("camera/temperatura", temperature)
            print(temperature)
            time.sleep(2)
                

    except KeyboardInterrupt:
        print('Ending...')
        mqttc.disconnect()

        timer.cancel()
        
        

if __name__ == '__main__':
    main()
