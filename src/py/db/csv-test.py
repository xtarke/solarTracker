#!/usr/bin/env python3

from influxdb import InfluxDBClient


def main():
    json_body = [
        {
            "measurement": "radiacao_media",
            "tags": {
                "tracker": "ufsc",
                "region": "florianopolis"
            },
            "fields": {
                "direta": -0.4,
                "global": -9.9,
                "difusa": -2.9
            }
        }
    ]

    client = InfluxDBClient('localhost', 8086)
    client.switch_database('solartracker')


    client.write_points(json_body)


    client.close()

if __name__ == '__main__':
    main()
