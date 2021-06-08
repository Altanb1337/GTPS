#pragma warning (disable : 4244)

#include "../../Server.hpp"

using namespace std;

GamePacket::~GamePacket() { delete[] data_; }

GamePacket::GamePacket(int delay, int netId) {
	len_ = 61;
	int messageType = 0x4, packetType = 0x1, charState = 0x8;

	memset(data_, 0, 61);
	memcpy(data_, &messageType, 4);
	memcpy(data_ + 4, &packetType, 4);
	memcpy(data_ + 8, &netId, 4);
	memcpy(data_ + 16, &charState, 4);
	memcpy(data_ + 24, &delay, 4);
}
GamePacket& GamePacket::extend(string str) {
	unsigned char* data = new unsigned char[len_ + 2 + str.length() + 4];
	memcpy(data, this->data_, len_);
	delete[] this->data_;

	this->data_ = data;
	data[len_] = index_;
	data[len_ + 1] = 0x2;
	
	unsigned long long len = str.length();
	memcpy(data + len_ + 2, &len, 4);
	memcpy(data + len_ + 6, str.data(), len);

	len_ += 2 + len + 4;
	index_++;
	data_[60] = static_cast<unsigned char>(index_);
	return *this;
}
GamePacket& GamePacket::extend(int i) {
	unsigned char* data = new unsigned char[len_ + 2 + 4];
	memcpy(data, this->data_, len_);
	delete[] this->data_;

	this->data_ = data;
	data[len_] = index_;
	data[len_ + 1] = 0x9;
	memcpy(data + len_ + 2, &i, 4);

	len_ += 2 + 4;
	index_++;
	data_[60] = static_cast<unsigned char>(index_);
	return *this;
}
GamePacket& GamePacket::extend(unsigned u) {
	unsigned char* data = new unsigned char[len_ + 2 + 4];
	memcpy(data, this->data_, len_);
	delete[] this->data_;

	this->data_ = data;
	data[len_] = index_;
	data[len_ + 1] = 0x5;
	memcpy(data + len_ + 2, &u, 4);

	len_ += 2 + 4;
	index_++;
	data_[60] = static_cast<unsigned char>(index_);
	return *this;
}
GamePacket& GamePacket::extend(float f) {
	unsigned char* data = new unsigned char[len_ + 2 + 4];
	memcpy(data, this->data_, len_);
	delete[] this->data_;

	this->data_ = data;
	data[len_] = index_;
	data[len_ + 1] = 0x1;
	memcpy(data + len_ + 2, &f, 4);

	len_ += 2 + 4;
	index_++;
	data_[60] = static_cast<unsigned char>(index_);
	return *this;
}
GamePacket& GamePacket::extend(float f, float f2) {
	unsigned char* data = new unsigned char[len_ + 2 + 8];
	memcpy(data, this->data_, len_);
	delete[] this->data_;

	this->data_ = data;
	data[len_] = index_;
	data[len_ + 1] = 0x3;
	memcpy(data + len_ + 2, &f, 4);
	memcpy(data + len_ + 6, &f2, 4);

	len_ += 2 + 8;
	index_++;
	data_[60] = static_cast<unsigned char>(index_);
	return *this;
}
GamePacket& GamePacket::extend(float f, float f2, float f3) {
	unsigned char* data = new unsigned char[len_ + 2 + 12];
	memcpy(data, this->data_, len_);
	delete[] this->data_;

	this->data_ = data;
	data[len_] = index_;
	data[len_ + 1] = 0x4;
	memcpy(data + len_ + 2, &f, 4);
	memcpy(data + len_ + 6, &f2, 4);
	memcpy(data + len_ + 10, &f3, 4);

	len_ += 2 + 12;
	index_++;
	data_[60] = static_cast<unsigned char>(index_);
	return *this;
}
GamePacket& GamePacket::sendData(ENetPeer* peer) {
	ENetPacket* packet = enet_packet_create(data_, len_, 1);
	enet_peer_send(peer, 0, packet);
	return *this;
}