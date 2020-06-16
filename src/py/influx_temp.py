import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
import time

import logging
logging.basicConfig(level=logging.DEBUG)

def on_connect(client, userdata, flags, rc):
    print("Connected to server: " + str(rc))
    client.subscribe("tanque/#")

def on_disconnect(client, userdata, rc):
    if rc != -1:
        print("Unexpected disconnection: " + str(rc))
        
def on_message_print(client, userdata, message):
    
    json_body = [
        {
            "measurement": "camera_temp",
            "tags": {              
                "region": "florianopolis"
            },
            # "time": "2018-11-18T09:35:00Z",
            "fields": {
                "Float_value": 0.64,
                #"value": 23.0
            }
        }
    ]

    # Add values to the databse
    topic_split = str(message.topic).split('/')
    payload_split = list(message.payload.decode('ASCII'))
    data_value = (''.join(payload_split).split(';',1))

    json_body[0]['measurement'] = 'temp' + str(topic_split[1]) + '-' + str(topic_split[3])
    json_body[0]['fields']['Float_value'] = float(data_value[0])

    # print(json_body[0])

    userdata.write_points(json_body)


    
def main():
    
    logger = logging.getLogger(__name__)
    
    client = InfluxDBClient('localhost', 8086)
    client.switch_database('temperaturas')
    
    mqttc = mqtt.Client()
    mqttc.on_disconnect = on_disconnect
    mqttc.enable_logger(logger)
    mqttc.on_connect = on_connect
    mqttc.on_message = on_message_print
    mqttc.user_data_set(client)
    mqttc.username_pw_set(username="      ", password="       ")

    mqttc.connect(host="cloudmqtt", port=     )  
    # mqttc.loop_start()
    
    
    mqttc.loop_forever()



    # print(json_body)   
    #try:
    #    while True:
    #        time.sleep(10)
            # print('Alive')
            # client.write_points(json_body)
            
               
    #except KeyboardInterrupt:
    #    print('Ending...')
    #    mqttc.disconnect()
    #    # client.close()

if __name__ == '__main__':
    main()

 
