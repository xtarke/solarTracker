/*
 * MqttComm.h
 *
 *  Created on: Jan 18, 2018
 *      Author: xtarke
 */

#ifndef MQTTCOMM_H_
#define MQTTCOMM_H_

#include <mosquittopp.h>

class MqttComm : public mosqpp::mosquittopp
{
public:
	MqttComm(const char *id, const char *host, int port);
	~MqttComm();

	void on_connect(int rc);
	void on_message(const struct mosquitto_message *message);
	void on_subcribe(int mid, int qos_count, const int *granted_qos);
};

#endif /* MQTTCOMM_H_ */
