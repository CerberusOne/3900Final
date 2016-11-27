#include <vector>
#include <string>
#include <Windows.h>
#include <stdint.h>
#include "Receive.h"
using namespace std;

vector<string> receiveBuffer;
size_t numAcks = 0;
size_t numError = 0;

void checkForEnq(HANDLE port)
{
	char buf;
	DWORD dwEvent, dwError;
	COMSTAT cs;

	//Event driven yay
	if (WaitCommEvent(port, &dwEvent, NULL))
	{
		ClearCommError(port, &dwError, &cs);
		if ((dwEvent & EV_RXCHAR) && cs.cbInQue)
		{
			if (!ReadFile(port, &buf, 1, NULL, NULL)) //Only want to read 1 byte (enq)
			{
				//Do nothing I guess?
			}
			else
			{
				writeACK(port);
				waitForPacket(port);
			}
		}
	}
}

void waitForPacket(HANDLE port)
{
	//Still need to set up timers
	char packet[1027];
	DWORD dwEvent, dwError;
	COMSTAT cs;
	string data;

	if (WaitCommEvent(port, &dwEvent, NULL))
	{
		ClearCommError(port, &dwError, &cs);
		if ((dwEvent & EV_RXCHAR) && cs.cbInQue)
		{
			if (!ReadFile(port, &packet, cs.cbInQue, NULL, NULL))
			{
				//Timer expire

			}
			else
			{
				data = packet;
				readPacket(data);
			}
		}
	}
}

bool readPacket(string packet)
{
	char syn = 22;

	if (packet.front() != syn) {	//If the first character isn't syn byte, that's an error
		return false;
	}

	string data(packet, 1);	//New string starting just after syn

							//Perform CRC on data
	if (checkError(data))
	{
		data.erase(data.end() - 2); //Remove crc bytes from data
		receiveBuffer.push_back(data);	//Starting to wonder if this goes here?
		return true;
	}

	numError++;
	return false;
}

bool writeACK(HANDLE port)
{
	char ack = 6;
	if (WriteFile(port, &ack, 1, NULL, NULL))
	{
		numAcks++;
		return true;
	}
	return false;
}

//I'll be honest, no idea if anything in here works
bool checkError(string data)
{
	uint8_t info[1026];	//fix magic number later, basically packet - syn
	uint16_t remainder = 000000000000000;

	for (size_t i = 0; i < data.size(); i++)
	{
		info[i] = data[i];
	}

	if (gen_crc16(info, 1026) != remainder)	//Full disclosure, no idea if this is setup right
	{
		return true;
	}

	return false;
}

// https://gist.github.com/jlamothe/2666368 -> crc code I used
uint16_t gen_crc16(const uint8_t *data, uint16_t size)
{
	uint16_t out = 0;
	int bits_read = 0, bit_flag;

	/* Sanity check: */
	if (data == NULL)
		return 0;

	while (size > 0)
	{
		bit_flag = out >> 15;

		/* Get next bit: */
		out <<= 1;
		out |= (*data >> (7 - bits_read)) & 1;

		/* Increment bit counter: */
		bits_read++;
		if (bits_read > 7)
		{
			bits_read = 0;
			data++;
			size--;
		}

		/* Cycle check: */
		if (bit_flag)
			out ^= CRC16;

	}
	return out;
}

bool endOfFile(string packet)
{
	bool eof = false;
	size_t location;


	if ((location = packet.find((char)0)) != string::npos)	//Look for null char
	{
		eof = true;
		receiveBuffer.push_back(packet.substr(0, location - 1));
	}

	receiveBuffer.push_back(packet);

	return eof;
}

void displayCurBuffer(HWND hwnd)
{
	string data;

	for (auto& x : receiveBuffer)
		data.append(x);

	//Textout to screen
}

void saveData() {
	//Select file to save to 

}

size_t getNumAcks()
{
	return numAcks;
}

size_t getNumErrors()
{
	return numError;
}

