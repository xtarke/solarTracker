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

	std::queue<int> zeSetHomeMessageQueue;
	std::queue<int> azSetHomeMessageQueue;

	/* App cmd topics */
	const std::string topics[5] = {"solar/cmd", "solar/cmd/azrepos" , "solar/cmd/zerepos",
									"solar/cmd/zesethome", "solar/cmd/azsethome"};

	/* Implementation of mosquittopp virtual functions */
	void on_connect(int rc);
	void on_message(const struct mosquitto_message *message);
	void on_subcribe(int mid, int qos_count, const int *granted_qos);


public:
	MqttComm(const char *id, const char *host, int port);
	MqttComm(const char *id, const char *host, int port, const char *user, const char *password);
	~MqttComm();

	int deQueueCmd();
	bool queueCmdIsEmpty() { return cmdMessageQueue.empty(); }

	int deQueueZe();
	bool queueZeIsEmpty() { return zeMessageQueue.empty(); }

	int deQueueAz();
	bool queueAzIsEmpty() { return azMessageQueue.empty(); }

	int deQueueZeHome();
	bool queueZeHomeIsEmpty() { return zeSetHomeMessageQueue.empty(); }

	int deQueueAzHome();
	bool queueAzHomeIsEmpty() { return azSetHomeMessageQueue.empty(); }

};

#endif /* MQTTCOMM_H_ */
