#pragma warning (disable : 4244)

#include "../../Player.hpp"

using namespace std;

int PlayerState::getState(ENetPeer* peer) {
	int i = 0;
	PlayerData* p = static_cast<PlayerData*>(peer->data);

	i |= p->noclip << 1;
	i |= p->doubleJump << 2;
	i |= p->invisible << 3;
	i |= p->noHands << 4;
	i |= p->noEyes << 5;
	i |= p->noBody << 6;
	i |= p->devilHorns << 7;
	i |= p->goldenHalo << 11;
	i |= p->isFrozen << 12;
	i |= p->cursed << 13;
	i |= p->hasCigar << 14;
	i |= p->isShining << 15;
	i |= p->isZombie << 16;
	i |= p->hitByLava << 17;
	i |= p->hauntedShadows << 18;
	i |= p->geigerRadiation << 19;
	i |= p->hasReflector << 20;
	i |= p->eggSkin << 21;
	i |= p->floatingPinapple << 22;
	i |= p->flyingPineapple << 23;
	i |= p->superSupporterName << 24;
	i |= p->superPineapple << 25;
	return i;
}

void PlayerState::sendState(ENetPeer* peer) {
	PlayerData* p = static_cast<PlayerData*>(peer->data);

	for (ENetPeer* currentPeer : inRange(Server)) {
		if (PlayerInfo::isHere(peer, currentPeer)) {
			PlayerMoving data;
			data.packetType = 0x14;
			data.characterState = 0;
			data.x = 1000; data.y = 100;
			data.punchX = 0; data.punchY = 0;
			data.xSpeed = 300; data.ySpeed = 600;
			data.netId = p->netId;
			data.plantingTree = getState(peer);

			unsigned char* raw = packPlayerMoving(&data);
			int var = 0x808000;
			float waterSpeed = 125.0f;

			memcpy(raw + 1, &var, 3);
			memcpy(raw + 16, &waterSpeed, 4);
			sendPacketRaw(currentPeer, 4, raw, 56, nullptr);
		}
	}
}

void PlayerState::sendData(ENetPeer* peer, PlayerMoving* data) {
	for (ENetPeer* currentPeer : inRange(Server)) {
		if (peer == currentPeer) continue;

		if (PlayerInfo::isHere(peer, currentPeer)) {
			data->netId = static_cast<PlayerData*>(peer->data)->netId;
			sendPacketRaw(currentPeer, 4, packPlayerMoving(data), 56, nullptr);
		}
	}
}

void PlayerState::sendNothing(ENetPeer* peer, int x, int y) {
	PlayerMoving data;
	data.netId = -1;
	data.packetType = 0x8;
	data.plantingTree = 0;
	data.x = x; data.y = y;
	data.punchX = x; data.punchY = y;
	sendPacketRaw(peer, 4, packPlayerMoving(&data), 56, nullptr);
}

void PlayerState::updateState(ENetPeer* peer) {
	sendState(peer);
	for (ENetPeer* currentPeer : inRange(Server)) {
		if (PlayerInfo::isHere(peer, currentPeer)) {
			sendState(currentPeer);
		}
	}
}