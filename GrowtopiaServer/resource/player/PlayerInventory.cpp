#pragma warning (disable : 4244)

#include "../../Player.hpp"

using namespace std;

void Inventory::sendInventory(ENetPeer* peer, PlayerInventory inventory) {
	unsigned long long inventoryLen = inventory.items.size();
	unsigned long long packetLen = 66 + inventoryLen * 4 + 4;
	unsigned char* data = new unsigned char[packetLen];
	int messageType = 0x4, packetType = 0x9, netId = -1, charState = 0x8;

	memset(data, 0, packetLen);
	memcpy(data, &messageType, 4);
	memcpy(data + 4, &packetType, 4);
	memcpy(data + 8, &netId, 4);
	memcpy(data + 16, &charState, 4);

	int inventoryVal = _byteswap_ulong(inventoryLen);
	memcpy(data + 66 - 4, &inventoryVal, 4);

	inventoryVal = _byteswap_ulong(inventory.size);
	memcpy(data + 66 - 8, &inventoryVal, 4);

	int val = 0;
	for (int i = 0; i < inventoryLen; i++) {
		val = 0;
		val |= inventory.items[i].itemId;
		val |= inventory.items[i].quantity << 16;
		val &= 0x00FFFFFF;
		val |= 0x00 << 24;
		memcpy(data + i * 4 + 66, &val, 4);
	}

	ENetPacket* packet = enet_packet_create(data, packetLen, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
	delete[] data;
}
void Inventory::addItem(ENetPeer* peer, unsigned short itemId, unsigned char quantity) {
	PlayerData* p = static_cast<PlayerData*>(peer->data);
	for (int i = 0; i < p->inventory.items.size(); i++) {
		if (p->inventory.items[i].itemId == itemId) {
			p->inventory.items[i].quantity += quantity;
			return;
		}
	}
	InventoryItem inventory;
	inventory.itemId = itemId;
	inventory.quantity = quantity;
	p->inventory.items.push_back(inventory);
}
void Inventory::removeItem(ENetPeer* peer, unsigned short itemId, unsigned char quantity) {
	PlayerData* p = static_cast<PlayerData*>(peer->data);
	for (int i = 0; i < p->inventory.items.size(); i++) {
		if (p->inventory.items[i].itemId == itemId) {
			if ((p->inventory.items[i].quantity - quantity) < 1)
				p->inventory.items.erase(p->inventory.items.begin() + i);
			else
				p->inventory.items[i].quantity -= quantity;
		}
	}
}
bool Inventory::searchItem(ENetPeer* peer, unsigned short itemId) {
	PlayerData* p = static_cast<PlayerData*>(peer->data);
	for (int i = 0; i < p->inventory.items.size(); i++) {
		if (p->inventory.items[i].itemId == itemId) {
			return true;
		}
	}
	return false;
}