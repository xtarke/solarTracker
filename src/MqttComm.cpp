/*
 * MqttComm.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: Renan Augusto Starke
 */

#include "MqttComm.h"

#include <iostream>

MqttComm::MqttComm(const char *id, const char *host, int port) {

	int ret;
	/* Initialize libmosquitto */
	mosqpp::lib_init();

	// threaded_set(true);

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
	std::cout << "Connected with code %d. \n";

	if (rc == 0) {
		ret = subscribe(NULL, "solar/cmd");

		std::cout << "subscribe: " << ret << "\n";
	}
}

void MqttComm::on_subcribe(int mid, int qos_count, const int *granted_qos)
{
	std::cout << "Subscription succeeded. \n";
}

void MqttComm::on_message(const struct mosquitto_message *message){

	std::cout << "Message  received: " << (*(char *)message->payload) << "\n";
}

