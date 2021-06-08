#include "../../Server.hpp"

using namespace std;

void Message::console(ENetPeer* peer, string message, int delay) {
	GamePacket packet(delay);
	packet.extend("OnConsoleMessage").extend(message).sendData(peer);
}
void Message::talkBubble(ENetHost* server, ENetPeer* peer, std::string message, int delay) {
	GamePacket packet(delay);
	packet.extend("OnTalkBubble").extend(static_cast<PlayerData*>(peer->data)->netId).extend(message);

	if (server == nullptr) {
		packet.sendData(peer);
		return;
	}
	for (ENetPeer* currentPeer : inRange(server)) {
		if (PlayerInfo::isHere(peer, currentPeer)) packet.sendData(currentPeer);
	}
}
void Message::dialog(ENetPeer* peer, string dialogFormat) {
	GamePacket packet;
	packet.extend("OnDialogRequest").extend(dialogFormat).sendData(peer);
}
void Message::talkBubble(ENetPeer* peer, ENetPeer* currentPeer, std::string message, int delay) {
	GamePacket packet(delay);
	packet.extend("OnTalkBubble").extend(static_cast<PlayerData*>(peer->data)->netId).extend(message).sendData(currentPeer);
}

void GameOutput::sendPositionSound(ENetHost* server, string file) {
	GamePacket packet;
	packet.extend("OnPlayPositioned").extend("audio/" + file);

	for (ENetPeer* currentPeer : inRange(server)) {
		packet.sendData(currentPeer);
	}
}
void GameOutput::sendSound(ENetPeer* peer, string file, ENetHost* server, bool sendToWorld, int delay) {
	string actionText = "action|play_sfx\nfile|audio/" + file + "\ndelayMS|" + to_string(delay) + "\n";
	unsigned char* data = new unsigned char[actionText.length() + 5];
	unsigned char u = 0;
	int type = 3;

	memcpy(data, &type, 4);
	memcpy(data + 4, actionText.data(), actionText.length());
	memcpy(data + 4 + actionText.length(), &u, 1);
	
	ENetPacket* packet = enet_packet_create(data, 5 + actionText.length(), ENET_PACKET_FLAG_RELIABLE);
	
	if (server == nullptr) {
		enet_peer_send(peer, 0, packet);
		return;
	}
	for (ENetPeer* currentPeer : inRange(server)) {
		if (PlayerInfo::isHere(peer, currentPeer) && sendToWorld) enet_peer_send(currentPeer, 0, packet);

		else enet_peer_send(currentPeer, 0, packet);
	}
}