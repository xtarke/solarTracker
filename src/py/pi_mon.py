import os
import time
from picamera import PiCamera
import paho.mqtt.client as mqtt

camera = PiCamera()

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection.")

def measure_temp():
    temp = os.popen("vcgencmd measure_temp").readline()
    return (temp.replace("temp=",""))

def on_message_print(client, userdata, message):
    print("%s %s" % (message.topic, message.payload))

    


    if (message.payload == b'1'):
        print("on");
    else:
        print("off")
        
        
    

def main():

    mqttc = mqtt.Client()

    mqttc.connect("192.168.0.10")
    mqttc.loop_start()
    mqttc.on_disconnect = on_disconnect
    mqttc.on_message = on_message_print

    mqttc.subscribe("camera/on") 

    try:
        while True:
            temperature = measure_temp()
            mqttc.publish("camera/temperatura", temperature)
            print(temperature)
            time.sleep(2)

    except KeyboardInterrupt:
        print('Ending...')


if __name__ == '__main__':
    main()
