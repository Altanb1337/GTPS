#include "../../Server.hpp"

using namespace std;

vector<ENetPeer*> inRange(ENetHost* server) {
	vector<ENetPeer*> peers;
	for (ENetPeer* currentPeer = Server->peers; currentPeer < &Server->peers[Server->peerCount]; ++currentPeer) {
		if (currentPeer->state != ENET_PEER_STATE_CONNECTED) continue;
		peers.push_back(currentPeer);
	}
	return peers;
}
vector<int> inRange(int endIndex) {
	vector<int> result;
	for (int i = 0; i < endIndex; i++) result.push_back(i);
	return result;
}