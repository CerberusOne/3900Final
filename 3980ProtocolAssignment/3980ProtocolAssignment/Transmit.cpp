#include <vector>
#include <string>
#include <iostream>
#include <Windows.h>
#include "MenuSettings.h"
#include "Constants.h"



void readCurrentDataFromBuffer();
void packetizeData();
void getCRC();
void sendPacket();
void resetTransmitBuffer();
void removeSentDataFromBuffer();
void generateRequestTimer(HWND hwnd);
void generateTransmitTimeout(HWND hwnd);

TIMERPROC requestingSend(HANDLE port, HWND hwnd) {
	DWORD written = 01;
	//char enq = 5;	
	char enq = 104;

	OutputDebugString("sending ENQ");
	if (WriteFile(port, &enq, 1, &written, NULL))
		OutputDebugString("wrote to port");
	else
		OutputDebugString("FAILED to write to port");
	
	//create timer that determines how long to wait before a request is considered a failure
	generateRequestTimer(hwnd);
	return NULL;
}

void readCurrentDataFromBuffer() {
	//this will take some sort of argument later that represents the data, not sure if string or char[] yet.
	packetizeData();
}

void packetizeData(/*Data*/) {
	//create packet

	//load syn bit into packet

	//load data into packet

	//pretty straight forward, get the crc value
	getCRC(/*data*/);

	//load crc into packet

	//send packet
	sendPacket(/*packet*/);
}

//this will be filled in with the external function
void getCRC() {

}

//main transmit loop happens here.
void sendPacket() {
	//this has to run no more than 3 times. It should keep track of how long the other side has to error check the data.
	//generateTransmitTimeout();
}

void resetTransmitBuffer() {
	/*
		should do something equivalent to buffer.popFront until the buffer is empty or hits a newfile (DC1) character.
	*/
}

void removeSentDataFromBuffer() {
	/*
		pop the front of the buffer, we succeded.
	*/
}

void generateRequestTimer(HWND hwnd) {
	//see request send for description
	SetTimer(hwnd, IDT_SEND, 960 , NULL);
}

void generateTransmitTimeout(HWND hwnd) {
	//see send packet
}
