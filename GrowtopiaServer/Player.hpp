#pragma once
#include "enet/enet.h"
#include "lib/json/json.hpp"
#include "Server.hpp"
#include "World.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <string>

struct InventoryItem {
	unsigned short itemId;
	unsigned char quantity;
};

struct PlayerInventory {
	std::vector<InventoryItem> items;
	unsigned short size = 100;
};
enum class AdminLevel {
	PLAYER, VIP, MOD, ADMIN, CM, CO_OWNER, OWNER
};
struct PlayerCloth {
	unsigned short hair = 0, shirt = 0, pants = 0,
		feet = 0, face = 0, hand = 0,
		back = 0, mask = 0, necklace = 0, ances = 0;
};
struct PlayerData {
	bool isIn = false, isAccount = false, radio = true, rotatedLeft = false, isUpdating = false,
		clothesIsUpdated = false, hasLogOn = false;
	int netId, userId, x, y, checkpointX, checkpointY, skinColor = 0x8295C3FF;
	int lastWrenchX, lastWrenchY;
	std::string tankName, ip, tankPass, guestName, rawName, roleName, displayName, country, currentWorld = "EXIT";
	AdminLevel adminLevel;
	PlayerCloth cloth;
	PlayerInventory inventory;

	bool noclip = false, doubleJump = false, invisible = false, noHands = false, noEyes = false,
		noBody = false, devilHorns = false, goldenHalo = false, isFrozen = false, cursed = false,
		ductTaped = false, hasCigar = false, isShining = false, isZombie = false, hitByLava = false,
		hauntedShadows = false, hasReflector = false, eggSkin = false, floatingPinapple = false,
		superSupporterName = false, superPineapple = false, isGhost = false, geigerRadiation = false,
		flyingPineapple = false;
};

struct PlayerMoving {
	int packetType, netId;
	float x, y;
	int characterState, plantingTree;
	float xSpeed, ySpeed;
	int punchX, punchY;
};

class PlayerInfo {
public:
	static void updateCloth(ENetPeer* peer);
	static inline bool isHere(ENetPeer* peer, ENetPeer* currentPeer) {
		return static_cast<PlayerData*>(peer->data)->currentWorld == static_cast<PlayerData*>(currentPeer->data)->currentWorld;
	}
	static bool isAccessed(ENetPeer* peer);
	static inline std::string getRoleName(std::string name) {
		std::ifstream ifs("players/" + name + ".json");
		nlohmann::json j;
		ifs >> j;
		ifs.close();
		return j["roleName"].get<std::string>();
	}
};
class PlayerState {
public:
	static void updateState(ENetPeer* peer);
	static int getState(ENetPeer* peer);
	static void sendState(ENetPeer* peer);
	static void sendData(ENetPeer* peer, PlayerMoving* data);
	static void sendNothing(ENetPeer* peer, int x, int y);
};
class PlayerAction {
public:
	static int login(ENetPeer* peer, std::string username, std::string password);
	static int registerAccount(ENetPeer* peer, std::string username, std::string password, std::string passwordVerify, std::string email, std::string discord);
	static void leaveWorld(ENetPeer* peer);
	static void sendChatMessage(ENetPeer* peer, std::string message);
	static void joinWorld(ENetPeer* peer, std::string world, int x = 0, int y = 0);
	static void sendAction(ENetPeer* peer, std::string action);
};
class Inventory {
public:
	static void sendInventory(ENetPeer* peer, PlayerInventory inventory);
	static void addItem(ENetPeer* peer, unsigned short itemId, unsigned char quantity);
	static void removeItem(ENetPeer* peer, unsigned short itemId, unsigned char quantity);
	static bool searchItem(ENetPeer* peer, unsigned short itemId);
};

inline unsigned char* packPlayerMoving(PlayerMoving* dataStruct) {
	unsigned char* data = new unsigned char[56];

	memset(data, 0, 56);
	memcpy(data, &dataStruct->packetType, 4);
	memcpy(data + 4, &dataStruct->netId, 4);
	memcpy(data + 12, &dataStruct->characterState, 4);
	memcpy(data + 20, &dataStruct->plantingTree, 4);
	memcpy(data + 24, &dataStruct->x, 4);
	memcpy(data + 28, &dataStruct->y, 4);
	memcpy(data + 32, &dataStruct->xSpeed, 4);
	memcpy(data + 36, &dataStruct->ySpeed, 4);
	memcpy(data + 44, &dataStruct->punchX, 4);
	memcpy(data + 48, &dataStruct->punchY, 4);
	return data;
}

inline PlayerMoving* unpackPlayerMoving(unsigned char* data) {
	PlayerMoving* dataStruct = new PlayerMoving;
	memcpy(&dataStruct->packetType, data, 4);
	memcpy(&dataStruct->netId, data + 4, 4);
	memcpy(&dataStruct->characterState, data + 12, 4);
	memcpy(&dataStruct->plantingTree, data + 20, 4);
	memcpy(&dataStruct->x, data + 24, 4);
	memcpy(&dataStruct->y, data + 28, 4);
	memcpy(&dataStruct->xSpeed, data + 32, 4);
	memcpy(&dataStruct->ySpeed, data + 36, 4);
	memcpy(&dataStruct->punchX, data + 44, 4);
	memcpy(&dataStruct->punchY, data + 48, 4);
	return dataStruct;
}

inline void sendPacketRaw(ENetPeer* peer, int a1, void* packetData, unsigned long long packetDataSize, void* a4, int packetFlag = ENET_PACKET_FLAG_RELIABLE) {
	if (a1 == 4 && *(static_cast<unsigned char*>(packetData) + 12) & 8) {
		int i = 4;
		ENetPacket* p = enet_packet_create(nullptr, packetDataSize + *(static_cast<unsigned long*>(packetData) + 13) + 5, packetFlag);
		memcpy(p->data, &i, 4);
		memcpy(reinterpret_cast<char*>(p->data) + 4, packetData, packetDataSize);
		memcpy(reinterpret_cast<char*>(p->data) + 4 + packetDataSize, a4, *(static_cast<unsigned long*>(packetData) + 13));
		enet_peer_send(peer, 0, p);
	}
	else {
		ENetPacket* p = enet_packet_create(nullptr, packetDataSize + 5, packetFlag);
		memcpy(p->data, &a1, 4);
		memcpy(reinterpret_cast<char*>(p->data) + 4, packetData, packetDataSize);
		enet_peer_send(peer, 0, p);
	}
	delete static_cast<char*>(packetData);
}