#include "../../World.hpp"

using namespace std;

WorldData* getWorld(ENetPeer* peer) {
	return WorldInfo::get2(static_cast<PlayerData*>(peer->data)->currentWorld).ptr;
}

WorldData generateWorld(string name) {
	WorldData world;
	world.name = name;
	world.items = new WorldItem[world.width * world.height];
	int randomDoor = rand() % 100;

	for (int i = 0; i < world.width * world.height; i++) {
		if (i >= 3700) {
			world.items[i].background = 14;
			world.items[i].foreground = 2;
		}
		if (i == 3600 + randomDoor)
			world.items[i].foreground = 6;
		else if (i >= 3600 && i < 3700)
			world.items[i].foreground = 0;
		if (i == 3700 + randomDoor)
			world.items[i].foreground = 8;
		
		if (i >= 3800 && i < 5400 && rand() % 48 == 0) 
			world.items[i].foreground = 10;
		if (i >= 5000 && i < 5400 && rand() % 6 == 0)
			world.items[i].foreground = 4;
		else if (i >= 5400)
			world.items[i].foreground = 8;
	}
	return world;
}