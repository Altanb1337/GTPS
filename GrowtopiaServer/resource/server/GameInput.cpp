#pragma warning (disable : 4244)
#pragma warning (disable : 4305)
#pragma warning (disable : 4309)
#pragma warning (disable : 4267)

#include "../../Server.hpp"

#include <sstream>

using namespace std;

string GameInput::upperCase(string str) {
	string result;
	for (char c : str) result += toupper(c);
	return result;
}
vector<string> GameInput::split(string delimeter, string content) {
	vector<string> result;
	int startIndex = 0, endIndex = 0;
	while ((endIndex = content.find(delimeter, startIndex)) < content.size()) {
		string val = content.substr(startIndex, endIndex - startIndex);
		result.push_back(val);
		startIndex = endIndex + delimeter.size();
	}
	if (startIndex < content.size())
		result.push_back(content.substr(startIndex));
	return result;
}
string GameInput::getRawString(string str) {
	string content, ret, ret2;

	for (char c : str) content += (c >= 'A' && c <= 'Z') ? c - ('A' - 'a') : c;

	for (int i = 0; i < content.length(); i++) {
		if (content[i] == '`') i++;

		else ret += content[i];
	}

	for (char c : ret) {
		if ((c >= 'a' && c <= ' z') || (c >= '0' && c <= ' 9')) ret2 += c;
	}
	
	return ret2;
}
string GameInput::filterColor(string str) {
	string ret;
	int colorLevel = 0;

	for (unsigned long long i = 0; i < str.length(); i++) {
		if (str[i] == '`') {
			ret += str[i];
			if ((i + 1) < str.length()) ret += str[i + 1];

			if ((i + 1) < str.length() && str[i + 1] == '`') colorLevel--;

			else colorLevel++;
			i++;
		}
		else ret += str[i];
	}
	for (int i = 0; i < colorLevel; i++) {
		ret += "``";
	}
	for (int i = 0; i > colorLevel; i--) {
		ret += '`w';
	}
	return ret;
}

void Dialog::catchReturnedDialog(ENetPeer* peer, string cch) {
	struct PlayerRegistration {
		bool activated = false;
		string name, password, passwordVerify, email, discord;
	};
	PlayerRegistration playerRegistration;

	struct SignEdit {
		bool activated = false;
		string message = "";
	};
	SignEdit signEdit;

	PlayerData* playerInfo = static_cast<PlayerData*>(peer->data);
	int xPos = playerInfo->lastWrenchX, yPos = playerInfo->lastWrenchY;
	WorldData* worldInfo = getWorld(peer);
	WorldItem worldItem = worldInfo->items[xPos + yPos * worldInfo->width];

	stringstream ss(cch);
	string to, button;
	while (getline(ss, to, '\n')) {
		vector<string> infoDat = GameInput::split("|", to);
		if (infoDat[0] == "buttonClicked") button = infoDat[1];

		else if (infoDat[0] == "dialog_name") {
			if (infoDat[1] == "AccountRegistration") {
				playerRegistration.activated = true;
			}
			else if (infoDat[1] == "sign_edit") {
				signEdit.activated = true;
			}
		}

		if (playerRegistration.activated) {
			if (infoDat[0] == "name")
				playerRegistration.name = infoDat[1];
			else if (infoDat[0] == "password")
				playerRegistration.password = infoDat[1];
			else if (infoDat[0] == "password_confirm")
				playerRegistration.passwordVerify = infoDat[1];
			else if (infoDat[0] == "email")
				playerRegistration.email = infoDat[1];
			else if (infoDat[0] == "discord")
				playerRegistration.discord = infoDat[1];
		}
		else if (signEdit.activated) {
			if (infoDat[0] == "message")
				signEdit.message = infoDat[1];
		}
	}
	if (playerRegistration.activated) {
		string errorMessage;

		switch (PlayerAction::registerAccount(peer, playerRegistration.name, playerRegistration.password, playerRegistration.passwordVerify, playerRegistration.email, playerRegistration.discord)) {
			case -1: case -2: errorMessage = "Invalid username";
				break;
			case -3: errorMessage = "Your password and confirmation password doesn't match each other";
				break;
			case -4: errorMessage = "Your email or discord username is invalid";
				break;
			case -5: errorMessage = "That account already exists";
				break;
		}
		if (!errorMessage.empty()) {
			Message::console(peer, "`4" + errorMessage);
			return;
		}

		Message::console(peer, "`oYour account has been successfully created! Disconnecting...");
		GameOutput::sendSound(peer, "piano_nice.wav");
		enet_peer_disconnect_later(peer, 0);
	}
	else if (signEdit.activated) {
		worldInfo->items[xPos + yPos * worldInfo->width].signMessage = signEdit.message;
	}
}