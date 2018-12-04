#!/usr/bin/env python3

# from influxdb import InfluxDBClient
import paho.mqtt.client as mqtt
import time
import logging
import argparse

logFilename = 'influxLight-' + time.strftime('%Y%m%d-%H-%M-%S') + '.log'
logging.basicConfig(filename=logFilename,level=logging.DEBUG)
light = 0

def on_connect(client, userdata, flags, rc):
    
    print('Connected with result code ' + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("$SYS/#")  

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print('Unexpected disconnection: ' + str(rc))
        logging.warning('Unexpected disconnection: ' + str(rc))
        
def on_message_print(client, userdata, message):
    global light
     
    my_string = message.payload.decode('ASCII')    
    light = int(my_string[0])*100 + int(my_string[1]) * 10 + int(my_string[2])
    
    # print(light)
    
def main():
    json_body = [
        {
            "measurement": "camera_temp",
            "tags": {
                "tracker": "home",
                "region": "florianopolis"
            },
            "time": "2018-11-18T09:35:00Z",
            "fields": {
                #"Float_value": 0.64,
                "value": 23.0
            }
        }
    ]
    
    parser = argparse.ArgumentParser(description='Simple mqtt to influx gateWay')
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
    mqttc.on_connect = on_connect

    mqttc.subscribe("teste/luz")
    logging.info('Subscribing to: ' + 'teste/luz')

    #client = InfluxDBClient(host='localhost', port=8086)
    #client.switch_database('solartracker')
       
    try:
        while True:
            # Check DST for influx timestamp
            if (time.daylight):     
                timezoneinfo = time.tzname[1]
            else:
                timezoneinfo = time.tzname[0]
                 
            time.sleep(10)
            json_body[0]['fields']['value'] = light
            json_body[0]['time'] = time.strftime('%Y-%m-%dT%H:%M:%S') + str(timezoneinfo) + ':00'
            
            print(json_body)
            # client.write_points(json_body)
            
               
    except KeyboardInterrupt:
        print('Ending...')
        mqttc.disconnect()
        # client.close()
         
            
if __name__ == '__main__':
    main()
