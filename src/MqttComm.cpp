/*
 * MqttComm.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: Renan Augusto Starke
 */

#include <iostream>
#include "MqttComm.h"

MqttComm::MqttComm(const char *id, const char *host, int port, const char *user, const char *password) {

	int ret;
	/* Initialize libmosquitto */
	mosqpp::lib_init();

	// threaded_set(true);
	username_pw_set(user, password);

	/* Connection to the broker */
	int keepalive = 120;
	ret = connect(host, port, keepalive);

	if (ret != MOSQ_ERR_SUCCESS){
		std::cerr << "Error connecting to mosquito broquer\n";
	}

	loop_start();
}

MqttComm::MqttComm(const char *id, const char *host, int port) {

	int ret;
	/* Initialize libmosquitto */
	mosqpp::lib_init();

	/* Connection to the broker */
	int keepalive = 120;
	ret = connect(host, port, keepalive);

	if (ret != MOSQ_ERR_SUCCESS){
		std::cerr << "Error connecting to mosquito broquer\n";
	}

	loop_start();
}

MqttComm::~MqttComm() {

	/*  Kill the thread and clean up */
	// loop_stop();
	mosqpp::lib_cleanup();
}

void MqttComm::on_connect(int rc)
{
	int ret;
	std::cout << "Connected with code " << rc << std::endl;

	if (rc == 0) {
		ret = subscribe(NULL, topics[0].c_str());
		ret = subscribe(NULL, topics[1].c_str());
		ret = subscribe(NULL, topics[2].c_str());
		ret = subscribe(NULL, topics[3].c_str());
		ret = subscribe(NULL, topics[4].c_str());

		std::cout << "subscribe: " << ret << std::endl;
	}
}

void MqttComm::on_subcribe(int mid, int qos_count, const int *granted_qos)
{
	std::cout << "Subscription succeeded. \n";
}

void MqttComm::on_message(const struct mosquitto_message *message){

	std::string topic = message->topic;
	std::string messageString;
	int cmd = 0;
	int zePos = 0;
	int azPos = 0;

	for (int i=0; i < message->payloadlen; i++){
		char c = ((char *)message->payload)[i];
		messageString.push_back(c);
	}

	if (topic.compare(topics[0]) == 0){
		try {
			cmd = std::stoi(messageString);
			cmdMessageQueue.push(cmd);
		}catch(const std::exception& e){
			std::cerr << "Mqqt cmd invalid" << std::endl;
		}
	}

	if (topic.compare(topics[1]) == 0){
		try {
			azPos = std::stoi(messageString);
			azMessageQueue.push(azPos);
		}catch(const std::exception& e){
			std::cerr << "Mqqt az invalid" << std::endl;
		}
	}


	if (topic.compare(topics[2]) == 0){
		try {
			zePos = std::stoi(messageString);
			zeMessageQueue.push(zePos);
		}catch(const std::exception& e){
			std::cerr << "Mqqt ze invalid" << std::endl;
		}
	}

	if (topic.compare(topics[3]) == 0){
		try {
			zePos = std::stoi(messageString);
			zeSetHomeMessageQueue.push(zePos);
		}catch(const std::exception& e){
			std::cerr << "Mqqt ze invalid" << std::endl;
		}
	}

	if (topic.compare(topics[4]) == 0){
		try {
			azPos = std::stoi(messageString);
			azSetHomeMessageQueue.push(azPos);
		}catch(const std::exception& e){
			std::cerr << "Mqqt az invalid" << std::endl;
		}
	}

	std::cout << "\tMQTT Topic: " << topic << std::endl;
	std::cout << "\tMQQT Message: " << messageString << std::endl;
}

int MqttComm::deQueueCmd(){

	int ret = cmdMessageQueue.front();
	cmdMessageQueue.pop();

	return ret;
}

int MqttComm::deQueueZe(){
	int ret = zeMessageQueue.front();
	zeMessageQueue.pop();

	return ret;
}

int MqttComm::deQueueAz(){
	int ret = azMessageQueue.front();
	azMessageQueue.pop();

	return ret;
}

int MqttComm::deQueueZeHome(){
	int ret = zeSetHomeMessageQueue.front();
	zeSetHomeMessageQueue.pop();

	return ret;
}

int MqttComm::deQueueAzHome(){
	int ret = azSetHomeMessageQueue.front();
	azSetHomeMessageQueue.pop();

	return ret;
}

