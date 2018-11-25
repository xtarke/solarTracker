from influxdb import InfluxDBClient
import paho.mqtt.client as mqtt
import time

temperature = '0'

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected disconnection.")
        
def on_message_print(client, userdata, message):
    global temperature
 
    # print("%s %s" % (message.topic, message.payload))    
    
    temperature = str(float(message.payload))
    #print(temperature)

    
def main():
    json_body = [
        {
            "measurement": "camera_temp",
            "tags": {
                "tracker": "ufsc",
                "region": "florianopolis"
            },
            "time": "2018-11-18T09:35:00Z",
            "fields": {
                "Float_value": 0.64,
                #"value": 23.0
            }
        }
    ]
    mqttc = mqtt.Client()

    mqttc.connect("150.162.29.60")
    mqttc.loop_start()
    mqttc.on_disconnect = on_disconnect
    mqttc.on_message = on_message_print

    mqttc.subscribe("camera/temperatura")
    
    print(json_body)

    client = InfluxDBClient(host='localhost', port=8086)
    client.switch_database('solartracker')
       
    try:
        while True:
            time.sleep(10)
            json_body[0]['fields']['Float_value'] = float(temperature)
            json_body[0]['time'] = time.strftime("%Y-%m-%dT%H:%M:%SZ")
            
            # print(json_body)
            
            client.write_points(json_body)
            
               
    except KeyboardInterrupt:
        print('Ending...')
        mqttc.disconnect()
        client.close()
         
            
if __name__ == '__main__':
    main()



