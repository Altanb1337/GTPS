#include "../../Server.hpp"

using namespace std;

unsigned char* getStructFromPacket(ENetPacket* packet) {
	unsigned long long packetLength = packet->dataLength;
	unsigned char* result = nullptr;

	if (packetLength >= 0x3C) {
		unsigned char* packetData = packet->data;
		result = packetData + 4;

		if (*reinterpret_cast<unsigned char*>(packetData + 16) & 8) {
			if (packetLength < (*reinterpret_cast<int*>(packetData + 56) + 60)) {
				result = 0;
			}
		}
		else {
			int i = 0;
			memcpy(packetData + 56, &i, 4);
		}
	}
	return result;
}
int getMessageTypeFromPacket(ENetPacket* packet) {
	int result;

	if (packet->dataLength > 3U) result = *(packet->data);

	else result = 0;

	return result;
}