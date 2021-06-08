#pragma warning (disable : 4244)
#define FMT_HEADER_ONLY

#include "../../lib/fmt/format.h"
#include "../../Player.hpp"
#include "../../Server.hpp"
#include "../../lib/json/json.hpp"
#include "../../Item.hpp"


#include <fstream>

using namespace std;
using json = nlohmann::json;

int PlayerAction::login(ENetPeer* peer, string username, string password) {
	if (ifstream("players/" + username + ".json").fail()) return -1;

	ifstream ifs("players/" + username + ".json");
	json j;
	ifs >> j;

	if (password != j["password"].get<string>()) return -1;

	for (ENetPeer* currentPeer : inRange(Server)) {
		if (peer == currentPeer) continue;

		if (static_cast<PlayerData*>(currentPeer->data)->rawName == username) {
			Message::console(currentPeer, "`oSomeone else has logged into this account.");
			enet_peer_disconnect_later(currentPeer, 0);
			Message::console(peer, "`oSomeone else was logged into this account! They were kicked out now.");
		}
	}
	return 1;
}
int PlayerAction::registerAccount(ENetPeer* peer, std::string username, std::string password, std::string passwordVerify, std::string email, std::string discord) {	
	if (username.empty() || username.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != string::npos) return -1;
	
	string name = GameInput::getRawString(username);
	if (name.length() < 3) return -2;

	if (password != passwordVerify) return -3;

	if (discord.find('#') == string::npos || email.find('@') == string::npos) return -4;

	if (ifstream("players/" + name + ".json").good()) return -5;

	json j;
	j["username"] = username;
	j["password"] = password;
	j["email"] = email;
	j["discord"] = discord;
	j["adminLevel"] = AdminLevel::PLAYER;
	j["roleName"] = username;
	j["userId"] = getRandomNumber(0, 65535);
	j["ip"] = static_cast<PlayerData*>(peer->data)->ip;
	j["worlds"] = json::array();
	j["xp"] = 0;
	j["level"] = 1;
	j["gem"] = 0;
	j["token"] = 0;

	j["punishment"]["muteLength"] = 0;
	j["punishment"]["curseLength"] = 0;
	j["punishment"]["banLength"] = 0;

	{
		json js = json::array();
		json x;

		x["itemId"] = 18;
		x["quantity"] = 1;
		js.push_back(x);
		
		x["itemId"] = 32;
		x["quantity"] = 1;
		js.push_back(x);

		j["inventory"] = js;
	}
	ofstream ofs("players/" + name + ".json");
	ofs << j;
	return 1;
}
void PlayerAction::leaveWorld(ENetPeer* peer) {
	PlayerData* p = static_cast<PlayerData*>(peer->data);
	if (p->currentWorld == "EXIT") return;

	GamePacket packet;
	packet.extend("OnRemove").extend("netID|" + to_string(p->netId) + "\n");

	for (ENetPeer* currentPeer : inRange(Server)) {
		if (PlayerInfo::isHere(peer, currentPeer)) {
			Message::console(currentPeer, "`5<`0" + p->displayName + "`5 left.>");
			Message::talkBubble(peer, currentPeer, "`5<`0" + p->displayName + "`5 left.>");
		}
	}
	p->currentWorld = "EXIT";
}
void PlayerAction::sendChatMessage(ENetPeer* peer, string message) {
	if (message.length() == 0) return;

	for (ENetPeer* currentPeer : inRange(Server)) {
		if (PlayerInfo::isHere(peer, currentPeer)) {
			Message::console(currentPeer, "CP:0_PL:4_OID:_CT:[W]_ `6<`0" + static_cast<PlayerData*>(peer->data)->displayName + "`6> `$" + message);
			Message::talkBubble(peer, currentPeer, message);
		}
	}
}

void PlayerAction::joinWorld(ENetPeer* peer, string world, int x, int y) {
	try {
		if (static_cast<PlayerData*>(peer->data)->currentWorld != "EXIT") leaveWorld(peer);
		
		if (world.length() > 30) return;

		WorldData worldInfo = WorldInfo::get(world);
		PlayerData* playerInfo = static_cast<PlayerData*>(peer->data);

		playerInfo->clothesIsUpdated = false;

		{
			string worldName = worldInfo.name;
			int width = worldInfo.width, height = worldInfo.height;
			int square = width * height;
			__int16 nameLen = static_cast<short>(worldName.length());
			int alloc = 8 * square;
			int total = 78 + nameLen + square + 24 + alloc;
			int s1 = 4, s3 = 8, zero = 0;
			unsigned char* data = new unsigned char[total];

			memset(data, 0, total);
			memcpy(data, &s1, 1);
			memcpy(data + 4, &s1, 1);
			memcpy(data + 16, &s3, 1);
			memcpy(data + 66, &nameLen, 1);
			memcpy(data + 68, worldName.data(), nameLen);
			memcpy(data + 68 + nameLen, &width, 1);
			memcpy(data + 72 + nameLen, &height, 1);
			memcpy(data + 76 + nameLen, &square, 2);

			ENetPacket* packet = enet_packet_create(data, total, ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, packet);

			for (int i = 0; i < square; i++) {
				int x = i % worldInfo.width, y = i / worldInfo.width;
				PlayerMoving data;
				data.packetType = 0x3;
				data.characterState = 0x0;
				data.x = x; data.y = y;
				data.punchX = x; data.punchY = y;
				data.xSpeed = 0;
				data.ySpeed = 0;
				data.netId = -1;

				if (worldInfo.items[i].foreground != 0) {
					data.plantingTree = worldInfo.items[i].foreground;
					sendPacketRaw(peer, 4, packPlayerMoving(&data), 56, nullptr);
				}
				if (worldInfo.items[i].background != 0) {
					data.plantingTree = worldInfo.items[i].background;
					sendPacketRaw(peer, 4, packPlayerMoving(&data), 56, nullptr);
				}
			}
			delete[] data;
		}
		playerInfo->currentWorld = worldInfo.name;

		int x = 3040, y = 736;
		for (int i = 0; i < worldInfo.width * worldInfo.height; i++) {
			if (getItem(worldInfo.items[i].foreground).blockType == BlockType::WHITE_DOOR) {
				x = (i % worldInfo.width) * 32;
				y = (i / worldInfo.width) * 32;
			}
		}
		{
			auto onSpawnFormat = [](ENetPeer* currentPeer, int x, int y) {
				PlayerData* playerInfo = static_cast<PlayerData*>(currentPeer->data);
				return fmt::format("spawn|avatar\nnetID|{}\nuserID|{}\ncolrect|0|0|20|30\nposXY|{}|{}|\nname|``{}\ncountry{}\ninvis|{}\nmstate|{}|\nsmstate|{}\ntype|local\n",
					playerInfo->netId, playerInfo->userId, x, y, playerInfo->displayName, playerInfo->country, 0, 0, 0);
			};
			{
				GamePacket packet;
				packet.extend("OnSpawn").extend(onSpawnFormat(peer, x, y)).sendData(peer);
			}
			for (ENetPeer* currentPeer : inRange(Server)) {
				if (peer != currentPeer) {
					if (PlayerInfo::isHere(peer, currentPeer)) {
						{
							GamePacket packet;
							packet.extend("OnSpawn").extend(onSpawnFormat(currentPeer, static_cast<PlayerData*>(currentPeer->data)->x, static_cast<PlayerData*>(currentPeer->data)->y)).sendData(peer);
						}
						{
							GamePacket packet;
							packet.extend("OnSpawn").extend(onSpawnFormat(peer, x, y));
						}
					}
				}
			}

			WorldTile::updateVisual(Server, peer);
		}
		Inventory::sendInventory(peer, static_cast<PlayerData*>(peer->data)->inventory);
	}
	catch (int e) {
		string message;
		switch (e) {
			case 1: message = "You have exited the world";
				break;
			case 2: message = "You have entered bad characters in the world name";
				break;
			case 3: message = "Exit from what? Click back if you're done playing";
				break;
			default: message = "I know this menu is magical and all, but it has its limitation!";
				break;
		}
		GamePacket packet;
		packet.extend("OnFailedToEnterWorld").extend(1).sendData(peer);
		Message::console(peer, message);
	}
}
void PlayerAction::sendAction(ENetPeer* peer, string action) {
	GamePacket packet(0, static_cast<PlayerData*>(peer->data)->netId);
	packet.extend("OnAction").extend(action);

	for (ENetPeer* currentPeer : inRange(Server)) {
		if (PlayerInfo::isHere(peer, currentPeer)) packet.sendData(currentPeer);
	}
}