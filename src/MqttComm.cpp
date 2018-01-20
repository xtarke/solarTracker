/*
 * MqttComm.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: Renan Augusto Starke
 */

#include "MqttComm.h"

#include <iostream>

MqttComm::MqttComm(const char *id, const char *host, int port) {

	/* Initialize libmosquitto */
	mosqpp::lib_init();

	// threaded_set(true);

	/* Connection to the broker */
	int keepalive = 120;
	connect(host, port, keepalive);

	loop_start();

}

MqttComm::~MqttComm() {

	/*  Kill the thread and clean up */
	loop_stop();
	mosqpp::lib_cleanup();
}

void MqttComm::on_connect(int rc)
{
	std::cout << "Connected with code %d. \n";

	if (rc == 0) {
		subscribe(NULL, "solar/cmd");
	}
}

void MqttComm::on_subcribe(int mid, int qos_count, const int *granted_qos)
{
	std::cout << "Subscription succeeded. \n";
}

void MqttComm::on_message(const struct mosquitto_message *message){

	std::cout << "Message  received: " << (*(char *)message->payload) << "\n";
}

