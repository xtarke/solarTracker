#!/usr/bin/env python3

import csv
from influxdb import InfluxDBClient

def csv_read(file_name) -> []:
    """ Returns a list of all CSV data."""    
    all_data = []
    print('Reading ' + file_name + ':')
    
    with open(file_name) as csv_file:        
        csv_reader = csv.reader(csv_file, delimiter=';')
        line_count = 0
        for row in csv_reader:
            if line_count == 0:
                # Skip header
                line_count += 1
            else:
                single_data = {
                    "measurement": "radiacao_media",
                    "tags": {
                        "tracker": "ufsc",
                        "region": "florianopolis"
                    },
                    "time": "2018-03-28T8:01:00Z",
                    "fields": {
                        "DNI": 0.0,
                        "GHI": 0.0,
                        "DHI": 0.0,
                        "temp": 0.0,
                        "UR": 0.0, 
                        "Press1": 0.0,
                        "Press2": 0.0,
                        "I0_normal": 0.0,
                        "I0_horizontal": 0.0,
                        "Zenith": 0.0,
                        "kt": 0.0,
                        "d": 0.0,
                        "kn": 0.0,
                        "kd": 0.0,
                        "Global_calc" :0.0
                    }
                }                
                
                single_data['fields']['DNI'] = float(row[1])
                single_data['fields']['GHI'] = float(row[2])
                single_data['fields']['DHI'] = float(row[3])
                single_data['fields']['temp'] = float(row[4])
                single_data['fields']['UR'] = float(row[5])
                single_data['fields']['Press1'] = float(row[6])
                single_data['fields']['Press2'] = float(row[7])
                single_data['fields']['I0_normal'] = float(row[8])
                single_data['fields']['I0_horizontal'] = float(row[9])
                single_data['fields']['Zenith'] = float(row[10])
                single_data['fields']['kt'] = float(row[11])
                single_data['fields']['d'] = float(row[12])
                single_data['fields']['kn'] = float(row[13])
                single_data['fields']['kd'] = float(row[14])
                single_data['fields']['Global_calc'] = float(row[15])
                
                # 2019-01-01 00:13:00.0 to "2018-03-28T8:01:00Z
                split_timestamp = (''.join(row[0]).split(' ',1))
                split_date = (''.join(split_timestamp[0]).split('/',2))

                time = split_date[2]+ '-' + split_date[1]+ '-' + split_date[0] + 'T' + split_timestamp[1] + 'Z'
                single_data['time'] = time

                # Ignore invalid data
                if ((single_data['fields']['DNI'] > -100) and (single_data['fields']['GHI'] > -100) and  (single_data['fields']['DHI'] > -100)):
                     all_data.append(single_data)
                else:
                    print('Ignore line: ' + str(line_count) + ' ' +  single_data['time'])

                line_count += 1
    print('Processed ' + str(line_count) + ' lines.')

    return all_data

def main():
    client = InfluxDBClient('localhost', 8086)
    client.switch_database('bsrn')

    my_data = csv_read('bsrn.csv')

    # Limit batch size
    client.write_points(my_data, batch_size=1000)
     
    client.close()
    print('Done!')

if __name__ == '__main__':
    main()
