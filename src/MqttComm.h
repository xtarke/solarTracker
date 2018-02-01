/*
 * MqttComm.h
 *
 *  Created on: Jan 18, 2018
 *      Author: xtarke
 */

#ifndef MQTTCOMM_H_
#define MQTTCOMM_H_

#include <queue>
#include <string>
#include <mosquittopp.h>

class MqttComm : public mosqpp::mosquittopp
{
private:

	std::queue<int> cmdMessageQueue;
	std::queue<int> zeMessageQueue;
	std::queue<int> azMessageQueue;

	/* App cmd topics */
	const std::string topics[3] = {"solar/cmd", "solar/cmd/azrepos" , "solar/cmd/zerepos"};

	/* Implementation of mosquittopp virtual functions */
	void on_connect(int rc);
	void on_message(const struct mosquitto_message *message);
	void on_subcribe(int mid, int qos_count, const int *granted_qos);


public:
	MqttComm(const char *id, const char *host, int port);
	~MqttComm();

	int deQueueCmd();
	bool queueCmdIsEmpty() { return cmdMessageQueue.empty(); }

	int deQueueZe();
	bool queueZeIsEmpty() { return zeMessageQueue.empty(); }

	int deQueueAz();
	bool queueAzIsEmpty() { return azMessageQueue.empty(); }

};

#endif /* MQTTCOMM_H_ */
