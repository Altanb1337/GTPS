#include "../../Server.hpp"
#include "../../Player.hpp"

bool PlayerInfo::isAccessed(ENetPeer* peer) {
	WorldData* world = getWorld(peer);
	return std::find(world->worldLock.admins.begin(), world->worldLock.admins.end(), static_cast<PlayerData*>(peer->data)->rawName) != world->worldLock.admins.end();
}
void PlayerInfo::updateCloth(ENetPeer* peer) {
	auto updateCloth = [](ENetPeer* thisPeer, ENetPeer* currentPeer) {
		PlayerCloth cloth = static_cast<PlayerData*>(thisPeer->data)->cloth;

		GamePacket packet(0, static_cast<PlayerData*>(thisPeer->data)->netId);
		packet.extend("OnSetClothing")
			.extend(cloth.hair, cloth.shirt, cloth.pants)
			.extend(cloth.feet, cloth.face, cloth.hand)
			.extend(cloth.back, cloth.mask, cloth.necklace)
			.extend(static_cast<PlayerData*>(thisPeer->data)->skinColor)
			.extend(cloth.ances, 0.0f, 0.0f).sendData(currentPeer);
	};

	for (ENetPeer* currentPeer : inRange(Server)) {
		if (isHere(peer, currentPeer)) {
			updateCloth(peer, currentPeer);
			updateCloth(currentPeer, peer);
		}
	}
}