#pragma once
#include <string>
#include <vector>
#include <iostream>

//Function prototypes
TIMERPROC requestingSend(HANDLE port, HWND hwnd);							//the timer function that handles transmit from connect

//Global Variable list
std::vector<std::string> transmitBuffer;						//buffer for trasnmitting data
int		failedRequests;											//Tracks how often we fail at getting permission to send
int		passedRequests;											//Tracks how often we succed at getting permission to send
int		sentPackets;											//Tracks how often we exit the send packet loop sucessfully
int		failedPackets;											//Tracks how often we exit the send packet loop unsucessfully
int		sentENQS;												//Tracks how often we ask for permission to send
int		recievedACKS;											//Tracks how often the other connection sends an ACK
bool	hasSendData;											//Tracks if we currently have data to send.
