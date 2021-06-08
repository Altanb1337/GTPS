#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "World.hpp"
#include "Server.hpp"
#include "enet/enet.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>

extern ENetHost* Server;
extern unsigned char* ItemsDatData;
extern int ItemsDatHash;
extern int ItemsDatSize;

unsigned char* getStructFromPacket(ENetPacket* packet);
int getMessageTypeFromPacket(ENetPacket* packet);

std::vector<ENetPeer*> inRange(ENetHost* server);
std::vector<int> inRange(int endIndex);

class GameInput {
public:
	static std::vector<std::string> split(std::string delimeter, std::string content);
	static std::string getRawString(std::string str);
	static std::string filterColor(std::string str);
	static std::string upperCase(std::string str);
};
class GameOutput {
public:
	static void sendPositionSound(ENetHost* server, std::string file);
	static void sendSound(ENetPeer* peer, std::string file, ENetHost* server = nullptr, bool sendToWorld = true, int delay = 0);
};
class DateTime {
public:
	static inline unsigned long long currentSeconds() {
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	// seconds - seconds since epoch
	static inline unsigned long long calculateSeconds(unsigned long long seconds) {
		return seconds - currentSeconds();
	}
	// seconds since epoch - seconds
	static inline unsigned long long calculateTimestamp(unsigned long long seconds) {
		return currentSeconds() - seconds;
	}
};
class Message {
public:
	static void console(ENetPeer* peer, std::string message, int delay = 0);
	static void dialog(ENetPeer* peer, std::string dialogFormat);
	// if server == null, sends to peer only
	static void talkBubble(ENetHost* server, ENetPeer* peer, std::string message, int delay = 0);
	static void talkBubble(ENetPeer* peer, ENetPeer* currentPeer, std::string message, int delay = 0);
};

enum class DialogInfo {
	BIG, SMALL
};

class Dialog {
	std::string str_ = "default|";
public:
	Dialog& endDialog(std::string dialogName, std::string cancelButton, std::string acceptButton);
	Dialog& addLabel(DialogInfo size, std::string label);
	Dialog& addTextbox(std::string text);
	Dialog& addSpacer(DialogInfo size);
	Dialog& addInput(bool isPassword, int length, std::string inputName, std::string label, std::string defaultInput);
	Dialog& addLabel(DialogInfo size, unsigned short itemId, std::string label);
	Dialog& addCustom(std::string format);
	static void catchReturnedDialog(ENetPeer* peer, std::string cch);

	inline std::string toString() {
		return str_;
	}
};

class GamePacket {
	int index_ = 0, len_ = 0;
	unsigned char* data_ = new unsigned char[61];
public:
	GamePacket(int delay = 0, int netId = -1);
	~GamePacket();
	GamePacket& extend(std::string str);
	GamePacket& extend(int i);
	GamePacket& extend(unsigned u);
	GamePacket& extend(float f);
	GamePacket& extend(float f, float f2);
	GamePacket& extend(float f, float f2, float f3);
	GamePacket& sendData(ENetPeer* peer);
};

inline void sendData(ENetPeer* peer, int num, char* data, int len) {
	char c = 0;
	ENetPacket* packet = enet_packet_create(nullptr, len + 5, ENET_PACKET_FLAG_RELIABLE);
	memcpy(packet->data, &num, 4);

	if (data != nullptr) memcpy(packet->data + 4, data, len);

	memcpy(packet->data + 4 + len, &c, 1);
	enet_peer_send(peer, 0, packet);
}
inline int getPacketId(char* data) {
	return *data;
}
inline char* getPacketData(char* data) {
	return data + 4;
}
inline std::string encodeText(char* text) {
	std::string ret;
	while (text[0] != 0) {
		switch (text[0]) {
			case '\n': ret.append("\\n");
				break;
			case '\t': ret.append("\\t");
				break;
			case '\b': ret.append("\\b");
				break;
			case '\\': ret.append("\\\\");
				break;
			case '\r': ret.append("\\r");
				break;
			default: ret += text[0];
				break;
		}
		text++;
	}
	return ret;
}
inline int charToInt(char c) {
	switch (c) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;

		case 'A': return 10;
		case 'B': return 11;
		case 'C': return 12;
		case 'D': return 13;
		case 'E': return 14;
		case 'F': return 15;

		default: break;
	}
}
inline char* getTextFromPacket(ENetPacket* packet) {
	char c = 0;
	memcpy(packet->data + packet->dataLength - 1, &c, 1);
	return reinterpret_cast<char*>(packet->data + 4);
}
inline std::ifstream::pos_type fileSize(char* fileName) {
	return std::ifstream(fileName, std::ifstream::ate | std::ifstream::binary).tellg();
}
inline int getRandomNumber(int min, int max) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(min, max);
	return distr(gen);
}
inline int countOnlinePeers(ENetHost* server) {
	int peers = 0;
	for (ENetPeer* currentPeer = server->peers; currentPeer < &server->peers[server->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
		peers++;
	}
	return peers;
}
inline unsigned hashString(unsigned char* str, int len) {
	unsigned acc = 0x55555555;
	unsigned char* n = str;

	if (len == 0) {
		while (*n) acc = (acc >> 27) + (acc << 5) + *n++;
	}
	else {
		for (int i : inRange(len)) acc = (acc >> 27) + (acc << 5) + *n++;
	}
	return acc;
}
inline unsigned char* getFileAttribute(std::string fileName, int* pSizeOut) {
	unsigned char* pData;
	FILE* file;
	errno_t err = fopen_s(&file, fileName.c_str(), "rb");

	if (!file) return nullptr;

	fseek(file, 0, SEEK_END);
	*pSizeOut = ftell(file);
	fseek(file, 0, SEEK_SET);

	pData = new unsigned char[((*pSizeOut) + 1)];
	if (!pData) return 0;

	pData[*pSizeOut] = 0;
	fread(pData, *pSizeOut, 1, file);
	fclose(file);

	return pData;
}
