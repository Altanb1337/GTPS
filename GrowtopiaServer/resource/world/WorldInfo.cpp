#pragma warning (disable : 4244)
#pragma warning (disable : 4267)

#include "../../World.hpp"
#include "../../lib/json/json.hpp"

#include <fstream>

using namespace std;
using json = nlohmann::json;

vector<WorldData> WorldInfo::worlds = {};

void WorldInfo::sendWorldOffer(ENetPeer* peer) {
	if (!static_cast<PlayerData*>(peer->data)->isIn) return;
	vector<WorldData> worlds = getRandomWorlds();
	string format = "default|";
	if (worlds.size() >= 1) format += worlds[0].name;

	format += "\nadd_button|Showing: Random Worlds|_catselect_|0.6|3529161471\n";

	for (int i = 0; i < worlds.size(); i++) {
		format += "add_floater|" + worlds[i].name + "|0|0.55|3529161471\n";
	}

	GamePacket packet;
	packet.extend("OnRequestWorldSelectMenu").extend(format).sendData(peer);
}

WorldData WorldInfo::get(string name) {
	return get2(name).info;
}

WorldPtr WorldInfo::get2(string name) {
	name = GameInput::upperCase(name);
	if (worlds.size() > 200) saveRedundant();

	for (int i : inRange(worlds.size())) {
		if (worlds[i].name == name) {
			WorldPtr ret;
			ret.id = i;
			ret.info = worlds[i];
			ret.ptr = &worlds[i];
			return ret;
		}
	}
	ifstream ifs("worlds/" + name + ".json");
	if (ifs.fail()) {
		WorldData info = generateWorld(name);
		worlds.push_back(info);

		WorldPtr ret;
		ret.info = info;
		ret.id = worlds.size() - 1;
		ret.ptr = &worlds[worlds.size() - 1];
		return ret;
	}
	json j;
	ifs >> j;
	WorldData world;
	world.name = j["name"].get<string>();
	world.worldLock.owner = j["owner"].get<string>();
	world.worldLock.isPublic = j["isPublic"];

	for (int i : inRange(world.width * world.height)) {
		world.items[i].foreground = j["tiles"][i]["foreground"];
		world.items[i].background = j["tiles"][i]["backgrund"];
	}

	worlds.push_back(world);

	WorldPtr ret;
	ret.id = worlds.size() - 1;
	ret.info = world;
	ret.ptr = &worlds[worlds.size() - 1];
	return ret;
}
void WorldInfo::flush(WorldData world) {
	json j;
	if (ifstream("worlds/" + world.name + ".json").good()) {
		ifstream ifs("worlds/" + world.name + ".json");
		ifs >> j;
	}
	j["name"] = world.name;
	j["owner"] = world.worldLock.owner;
	j["isPublic"] = world.worldLock.isPublic;
	
	json js = json::array();
	for (int i : inRange(world.width * world.height)) {
		json x;
		x["foreground"] = world.items[i].foreground;
		x["background"] = world.items[i].background;

		js.push_back(x);
	}
	j["tiles"] = js;

	ofstream ofs("worlds/" + world.name + ".json");
	ofs << j;
}
void WorldInfo::flush(WorldPtr world) {
	flush(world.info);
}
void WorldInfo::save(WorldPtr world) {
	flush(world);
	delete world.info.items;
	worlds.erase(worlds.begin() + world.id);
}
void WorldInfo::saveAll() {
	for (int i : inRange(worlds.size())) {
		flush(worlds[i]);
		delete[] worlds[i].items;
	}
	worlds.clear();
}
vector<WorldData> WorldInfo::getRandomWorlds() {
	vector<WorldData> ret;
	for (int i : inRange((worlds.size() < 10) ? worlds.size() : 10)) ret.push_back(worlds[i]);

	if (worlds.size() > 4) {
		for (int i : inRange(6)) {
			bool isPossible = true;
			WorldData world = worlds[rand() % (worlds.size() - 4)];
			
			for (int j : inRange(ret.size())) {
				if (world.name == ret[i].name || world.name == "EXIT") isPossible = false;
			}
			if (isPossible) ret.push_back(world);
		}
	}
	return ret;
}
void WorldInfo::saveRedundant() {
	for (int i = 4; i < worlds.size(); i++) {
		bool canFree = true;
		
		for (ENetPeer* currentPeer : inRange(Server)) {
			if (static_cast<PlayerData*>(currentPeer->data)->currentWorld == worlds[i].name) canFree = false;
		}
		if (canFree) {
			flush(worlds[i]);
			delete worlds[i].items;
			worlds.erase(worlds.begin() + i);
			i--;
		}
	}
}