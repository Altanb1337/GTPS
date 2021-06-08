#define FMT_HEADER_ONLY
#pragma warning (disable : 4244)

#include "../../lib/fmt/format.h"
#include "../../World.hpp"
#include "../../Item.hpp"
#include "../../Server.hpp"

using namespace std;

void WorldTile::sendRoulette(ENetHost* server, ENetPeer* peer) {
	int number = getRandomNumber(0, 37);
	string numberCode = ((number % 2) == 0) ? "b" : "4";
	numberCode = number == 0 ? "2" : numberCode;
	
	string message = "`5[`0" + static_cast<PlayerData*>(peer->data)->displayName + "`o spun the wheel and got `" + numberCode + to_string(number) + "`o!`5]";
	for (ENetPeer* currentPeer : inRange(server)) {
		if (PlayerInfo::isHere(peer, currentPeer)) {
			Message::console(currentPeer, message, 2000);
			Message::talkBubble(peer, currentPeer, message, 2000);
		}
	}
}

void WorldTile::sendSign(ENetHost* server, ENetPeer* peer, int x, int y, string message) {
	WorldData world = WorldInfo::get(static_cast<PlayerData*>(peer->data)->currentWorld);
	unsigned short foreground = world.items[x + y * world.width].foreground,
		background = world.items[x + y * world.width].background;

	PlayerMoving playerMove;
	playerMove.packetType = 0x3;
	playerMove.characterState = 0x0;
	playerMove.x = x; playerMove.y = y;
	playerMove.punchX = x; playerMove.punchY = y;
	playerMove.xSpeed = 0; playerMove.ySpeed = 0;
	playerMove.netId = -1;
	playerMove.plantingTree = world.items[x + y * world.width].foreground;
	sendPacketRaw(peer, 4, packPlayerMoving(&playerMove), 56, nullptr);

	int sizeLen = message.length() + 15 + 52;

	unsigned char* data = new unsigned char[sizeLen + 4];
	memset(data, 0, sizeLen + 4);

	ENetPacket* packet = enet_packet_create(nullptr, sizeLen + 4 + 5, ENET_PACKET_FLAG_RELIABLE);

	int var = 5, var2 = 8, var3 = 15 + message.length(),
		var4 = 0, var5 = 2, var6 = message.length(), var7 = -1,
		var8 = 4;
	int dataIndex = sizeLen - 8 - (message.length() - 15);

	memcpy(data, &var, 1);
	memcpy(data + 8 + 3 + 1, &var2, 4);
	memcpy(data + dataIndex, &x, 4);
	memcpy(data + dataIndex + 4, &y, 4);
	memcpy(data + 4 + dataIndex + 4, &var3, 4);
	memcpy(data + 56, &foreground, 2);
	memcpy(data + 56 + 2, &background, 2);
	memcpy(data + 4 + 56, &var4, 4);
	memcpy(data + 56 + 4 + 4, &var5, 1);
	memcpy(data + 56 + 4 + 5, &var6, 2);
	memcpy(data + 2 + 56 + 4 + 5, message.data(), message.length());
	memcpy(data + 52 + 15 + message.length(), &var7, 4);
	memcpy(packet->data, &var8, 4);

	memcpy(reinterpret_cast<char*>(packet->data) + 4, data, sizeLen + 4);

	if (server == nullptr) {
		enet_peer_send(peer, 0, packet);
		delete[] data;
		return;
	}
	for (ENetPeer* currentPeer : inRange(server)) {
		if (PlayerInfo::isHere(peer, currentPeer)) {
			enet_peer_send(currentPeer, 0, packet);
		}
	}
	delete[] data;
}

void WorldTile::updateTile(ENetHost* server, ENetPeer* peer, int x, int y, unsigned short tile) {
	PlayerState::sendNothing(peer, x, y);

	PlayerMoving data;
	data.packetType = 0x3;
	data.characterState = 0x0;
	data.x = x; data.y = y;
	data.punchX = x; data.punchY = y;
	data.xSpeed = 0; data.ySpeed = 0;
	data.netId = static_cast<PlayerData*>(peer->data)->netId;
	data.plantingTree = tile;

	WorldData* world = getWorld(peer);
	WorldItem worldItem = world->items[x + y * world->width];
	unsigned short blockId = world->items[x + y * world->width].foreground == 0 ?
		world->items[x + y * world->width].background : world->items[x + y * world->width].foreground;

	ItemDefinition tileInfo = getItem(tile), blockInfo = getItem(blockId);
	PlayerData* playerInfo = static_cast<PlayerData*>(peer->data);

	if (tileInfo.blockType == BlockType::CLOTHING) return;

	if (blockInfo.blockType == BlockType::BEDROCK && playerInfo->adminLevel < AdminLevel::MOD) {
		Message::talkBubble(peer, nullptr, "It's too heavy to break.");
		return;
	}
	else if (tileInfo.blockType == BlockType::BEDROCK && playerInfo->adminLevel < AdminLevel::MOD) {
		Message::talkBubble(peer, nullptr, "It's too heavy to place.");
		return;
	}

	if (tile == 32) {
		playerInfo->lastWrenchX = x;
		playerInfo->lastWrenchY = y;
	}

	switch (blockInfo.blockType) {
		case BlockType::WORLD_LOCK: 
		{
			if (tile == 32) break;

			std::string worldAccess = "`4No access";
			if (world->worldLock.isPublic) worldAccess = "`$Open for Public";

			else if (PlayerInfo::isAccessed(peer) || playerInfo->adminLevel >= AdminLevel::CM || world->worldLock.owner == playerInfo->rawName)
				worldAccess = "`2Access granted";

			Message::talkBubble(peer, peer, fmt::format("`0{}`o's `${}`o. ({}`o.)", PlayerInfo::getRoleName(world->worldLock.owner), blockInfo.name, worldAccess));
		} break;
		case BlockType::SIGN:
		{
			if (tile != 32) break;

			Dialog dialog;
			dialog.addLabel(DialogInfo::BIG, blockId, "Edit " + blockInfo.name)
				.addTextbox("`oWhat would you like to write on this sign?")
				.addInput(false, 100, "message", "", worldItem.signMessage)
				.endDialog("sign_edit", "Close", "Edit");
			Message::dialog(peer, dialog.toString());
		} break;
	}
	if (!PlayerInfo::isAccessed(peer) && world->worldLock.owner != playerInfo->rawName && playerInfo->adminLevel < AdminLevel::CM) {
		GameOutput::sendPositionSound(Server, "punch_locked.wav");
		return;
	}

	if (tile == 18) {
		data.packetType = 0x8;
		data.plantingTree = 6;
		if ((DateTime::currentSeconds() - worldItem.breakTime) >= 4000) {
			world->items[x + y * world->width].breakTime = DateTime::currentSeconds();
			world->items[x + y * world->width].breakLevel = 0;

			if (worldItem.foreground == 758) sendRoulette(Server, peer);
		}

		world->items[x + y * world->width].breakTime = DateTime::currentSeconds();
		world->items[x + y * world->width].breakLevel += 6;

		if (worldItem.breakLevel >= (getItem(blockId).breakHits * 6)) {
			data.packetType = 0x3;
			data.plantingTree = 18;
			world->items[x + y * world->width].breakLevel = 0;

			if (worldItem.foreground == 0) world->items[x + y * world->width].background = 6864;

			else {
				world->items[x + y * world->width].foreground = 0;

				switch (getItem(blockId).blockType) {
					case BlockType::WORLD_LOCK: 
					{
						string message = fmt::format("`5[`0{}`o has had its `${}`o removed!`5]", world->name, blockInfo.name);

						for (ENetPeer* currentPeer : inRange(Server)) {
							if (PlayerInfo::isHere(peer, currentPeer)) {
								Message::console(currentPeer, message);
								Message::talkBubble(peer, currentPeer, message);
							}
						}
					} break;
				}
			}
		}
	}
	else {
		if (!Inventory::searchItem(peer, tile)) return;

		Inventory::removeItem(peer, tile, 1);

		if (blockInfo.blockType == BlockType::BACKGROUND) world->items[x + y * world->width].background = tile;

		else {
			if (world->items[x + y * world->width].foreground != 0) return;
			world->items[x + y * world->width].foreground = tile;

			if (tile == 242) {
				world->worldLock.owner = playerInfo->rawName;
				world->worldLock.isPublic = false;

				string message = fmt::format("`3[`0{}`o has been `$World Locked `oby `0{}!`3]", world->name, playerInfo->displayName);
				for (ENetPeer* currentPeer : inRange(Server)) {
					if (PlayerInfo::isHere(peer, currentPeer)) {
						Message::console(currentPeer, message);
						Message::talkBubble(peer, currentPeer, message);
					}
				}
			}
		}
		world->items[x + y * world->width].breakLevel = 0;
	}

	for (ENetPeer* currentPeer : inRange(Server)) {
		if (PlayerInfo::isHere(peer, currentPeer))
			sendPacketRaw(currentPeer, 4, packPlayerMoving(&data), 56, nullptr);
	}
}

void WorldTile::updateVisual(ENetHost* server, ENetPeer* peer) {
	WorldData* world = getWorld(peer);

	for (int i = 0; i < world->width * world->height; i++) {
		int x = i % world->width, y = i / world->width;
		WorldItem worldItem = world->items[i];
		switch (getItem(world->items[i].foreground).blockType) {
			case BlockType::SIGN:
			{
				if (worldItem.signMessage.empty()) break;
				
				sendSign(nullptr, peer, x, y, worldItem.signMessage);
			} break;
			default: break;
		}
	}
}
