
#ifndef RECEIVE_H
#define RECEIVE_H

#include <string>
#include <Windows.h>
#include <stdint.h>

#define CRC16 0x8005

void checkForEnq(HANDLE port);
void waitForPacket(HANDLE port);
bool readPacket(std::string packet);
bool writeACK(HANDLE port);
bool checkError(std::string data);
uint16_t gen_crc16(const uint8_t *data, uint16_t size);
bool endOfFile(std::string packet);
void displayCurBuffer(HWND hwnd);

#endif