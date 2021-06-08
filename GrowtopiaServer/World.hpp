#pragma once
#include "Server.hpp"
#include "Player.hpp"
#include "enet/enet.h"

#include <vector>
#include <string>

struct WorldItem {
	unsigned short foreground = 0, background = 0, breakLevel = 0;
	unsigned long long breakTime = 0;

	std::string signMessage = "";
};
struct World_WorldLock {
	std::string owner = "";
	bool isPublic = false, isJammed = false, silenced = false;
	std::vector<std::string> admins;
};
struct WorldData {
	int width = 100, height = 60;
	std::string name = "TEST";
	WorldItem* items;
	World_WorldLock worldLock;
};
struct WorldPtr {
	WorldData* ptr;
	WorldData info;
	int id;
};
class WorldInfo {
	static std::vector<WorldData> worlds;
public:
	static WorldPtr get2(std::string name);
	static WorldData get(std::string name);
	static void flush(WorldData info);
	static void flush(WorldPtr info);
	static void save(WorldPtr info);
	static void saveAll();
	static void saveRedundant();
	static std::vector<WorldData> getRandomWorlds();
	static void sendWorldOffer(ENetPeer* peer);
};

WorldData generateWorld(std::string worldName);

WorldData* getWorld(ENetPeer* peer);

class WorldTile {
public:
	static void updateVisual(ENetHost* server, ENetPeer* peer);
	static void sendSign(ENetHost* server, ENetPeer* peer, int x, int y, std::string message);
	static void sendRoulette(ENetHost* server, ENetPeer* peer);
	static void updateTile(ENetHost* server, ENetPeer* peer, int x, int y, unsigned short tile);
};