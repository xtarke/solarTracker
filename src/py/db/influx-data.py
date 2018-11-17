from influxdb import InfluxDBClient

json_body = [
    {
        "measurement": "camera_temp",
        "tags": {
            "tracker": "ufsc",
            "region": "florianopolis"
        },
        "time": "2018-11-17T23:25:00Z",
        "fields": {
            "value": 50
        }
    }
]
        
client = InfluxDBClient(host='localhost', port=8086)

client.switch_database('solartracker')

client.write_points(json_body)

