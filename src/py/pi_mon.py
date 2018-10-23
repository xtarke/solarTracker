import os
import time
import threading
from picamera import PiCamera
import paho.mqtt.client as mqtt


cameraOn = False

camera = PiCamera(resolution=(800, 600))

class MyThread(threading.Thread):
    def run(self):
        print("{} started!".format(self.getName()))              # "Thread-x started!"
        time.sleep(1)                                      # Pretend to work for a second
        print("{} finished!".format(self.getName()))             # "Thread-x finished!"

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
            

def main():

    mqttc = mqtt.Client()

    mqttc.connect("192.168.0.10")
    mqttc.loop_start()
    mqttc.on_disconnect = on_disconnect
    mqttc.on_message = on_message_print

    mqttc.subscribe("camera/on")

    mythread = MyThread(name = "Camera_Thread")
                           

    try:
        while True:
            temperature = measure_temp()
            mqttc.publish("camera/temperatura", temperature)
            print(temperature)
            time.sleep(2)

            if (cameraOn == True):
                mythread.start()

    except KeyboardInterrupt:
        print('Ending...')
        mqttc.disconnect()
        camera.close()
        

if __name__ == '__main__':
    main()
