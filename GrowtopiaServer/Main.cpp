#include "Server.hpp"
#include "Item.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "lib/json/json.hpp"
#include "enet/enet.h"

#include <iostream>
#include <signal.h>
#include <csignal>
#include <sstream>

using namespace std;

int main() {
	printf("Growtopia Private Server. Initializing enet server...\n");

	enet_initialize();
	signal(2, [](int i) {
		WorldInfo::saveAll();
		exit(0);
	});

	ENetAddress address;
	enet_address_set_host(&address, "0.0.0.0");
	address.port = 17091;

	Server = enet_host_create(&address, 1024, 10, 0, 0);

	if (Server == nullptr) {
		fprintf(stderr, "An error has occured while trying to create an ENet server host.\n");
		exit(EXIT_FAILURE);
	}
	Server->checksum = enet_crc32;
	enet_host_compress_with_range_coder(Server);
	printf("ENet server initialized.\nBuilding items database...\n");
	buildItemDatabase();
	printf("Items database build : %d items found.\nitems.dat hash : %d\n", static_cast<int>(itemDefs.size()), ItemsDatHash);
	ENetEvent event;
	while (true) {
		while (enet_host_service(Server, &event, 1000)) {
			ENetPeer* peer = event.peer;
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
				{
					event.peer->data = new PlayerData;
					char clientConnection[16];
					enet_address_get_host_ip(&peer->address, clientConnection, 16);
					static_cast<PlayerData*>(peer->data)->ip = clientConnection;
					static_cast<PlayerData*>(peer->data)->netId = getRandomNumber(0, getRandomNumber(100, 1000));

					sendData(peer, 1, nullptr, 0);
				} break;
				case ENET_EVENT_TYPE_RECEIVE:
				{
					if (static_cast<PlayerData*>(peer->data)->isUpdating) {
						break;
					}

					int messageType = getMessageTypeFromPacket(event.packet);
					WorldData* world = getWorld(peer);

					switch (messageType) {
						case 2:
						{
							string cch = getTextFromPacket(event.packet);
							string str = cch.substr(cch.find("text|") + 5, cch.length() - (cch.find("text|") - 1));

							if (cch.find("action|wrench") != string::npos) {

							}
							else if (cch.find("action|setSkin") != string::npos) {

							}
							else if (cch.find("action|respawn") != string::npos) {

							}
							else if (cch.find("action|store") != string::npos) {
								if (!static_cast<PlayerData*>(peer->data)->isAccount) {
									Dialog dialog;
									dialog.addLabel(DialogInfo::BIG, 206, "Get a GrowID")
										.addSpacer(DialogInfo::SMALL)
										.addTextbox("`oBy choosing a `0GrowID`o, you can use a name and password to logon from any device. Your `0name`o will be shown to other players!")
										.addSpacer(DialogInfo::SMALL)
										.addInput(false, 16, "name", "Name", "")
										.addTextbox("`oYour `0password`o must contain `00 to `010`o characters.")
										.addInput(true, 18, "password", "Password", "")
										.addInput(true, 18, "password_confirm", "Password Verify", "")
										.addTextbox("`oYour `0email`o will only be used for account verification and support. If you enter a fake email, you can't verify your account, recover or change your password.")
										.addInput(false, 100, "email", "Email", "")
										.addInput(false, 100, "discord", "Discord", "")
										.addTextbox("`oWe will never ask for your password or email. Never share it with anyone!")
										.endDialog("AccountRegistration", "Cancel", "Get My GrowID");
									Message::dialog(peer, dialog.toString());
								}
							}
							else if (cch.find("action|dialog_return") != string::npos) {

							}
							else if (cch.find("text|") != string::npos) {
								if (str.length() >= 1 && str[0] == '/') {
									PlayerAction::sendAction(peer, str);
								}
								else if (str.length() >= 1) {
									PlayerAction::sendChatMessage(peer, str);
								}
							}
							if (!static_cast<PlayerData*>(peer->data)->isIn) {
								{
									GamePacket p;
									p.extend("OnSuperMainStartAcceptLogonHrdxs47254722215a")
										.extend(ItemsDatHash).extend("ubistatic-a.akamaihd.net")
										.extend(17091).extend("cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster")
										.extend("proto=84|choosemusic=audio/mp3/about_theme.mp3|active_holiday=0|server_tick=226933875|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|")
										.sendData(peer);
								}
								stringstream ss(getTextFromPacket(event.packet));
								string to;
								PlayerData* p = static_cast<PlayerData*>(peer->data);
								while (getline(ss, to, '\n')) {
									string id = to.substr(0, to.find('|')),
										act = to.substr(to.find('|') + 1, to.length() - (to.find('|') - 1));
								
									if (id == "tankIDName") {
										p->tankName = act;
										p->isAccount = true;
									}
									else if (id == "tankIDPass") p->tankPass = act;

									else if (id == "requestedName") p->guestName = act;

									else if (id == "country") p->country = act;
								}
								if (!p->isAccount) {
									p->hasLogOn = true;
									p->rawName = "";
									p->displayName = "`0[`2GUEST`0] " + GameInput::filterColor(p->guestName.substr(0, p->guestName.length() > 15 ? 15 : p->guestName.length()));
								}
								else {
									p->rawName = GameInput::getRawString(p->tankName);
									int logStatus = PlayerAction::login(peer, p->rawName, p->tankPass);

									if (logStatus == -1) {
										Message::console(peer, "`oWrong username or password.");
										enet_peer_disconnect_later(peer, 0);
									}
									else {
										Message::console(peer, "`2Successfully logged on.");
									}
								}
								for (char c : p->displayName) if (c < 0x20 || c > 0x7A) {
									Message::console(peer, "`4Bad characters in name.");
									enet_peer_disconnect_later(peer, 0);
								}

								{
									GamePacket packet;
									packet.extend("SetHasGrowID").extend(p->isAccount).extend(p->tankName).extend(p->tankPass).sendData(peer);
								}
							}
							if (cch.substr(0, 17) == "action|enter_game" && !static_cast<PlayerData*>(peer->data)->isIn) {
								static_cast<PlayerData*>(peer->data)->isIn = true;
								static_cast<PlayerData*>(peer->data)->hasLogOn = true;

								WorldInfo::sendWorldOffer(peer);

								for (int i = 0; i < 200; i++) Inventory::addItem(peer, i * 2 + 2, 200);
							}
							if (strcmp(getTextFromPacket(event.packet), "action|refresh_item_data\n") == 0) {
								ENetPacket* packet = enet_packet_create(ItemsDatData, ItemsDatSize + 60, ENET_PACKET_FLAG_RELIABLE);
								enet_peer_send(peer, 0, packet);
								//enet_peer_disconnect_later(peer, 0);
								static_cast<PlayerData*>(peer->data)->isUpdating = true;
							}
						} break;
						case 3:
						{
							stringstream ss(getTextFromPacket(event.packet));
							string to;
							bool isJoinReq = false;
							while (getline(ss, to, '\n')) {
								string id = to.substr(0, to.find('|')), act = to.substr(to.find('|') + 1, to.length() - (to.find('|') - 1));

								if (id == "name" && isJoinReq) {
									if (!static_cast<PlayerData*>(peer->data)->hasLogOn) break;
									PlayerAction::joinWorld(peer, GameInput::upperCase(act));
								}
								else if (id == "action") {
									if (act == "join_request") isJoinReq = true;

									else if (act == "quit_to_exit") {
										PlayerAction::leaveWorld(peer);
										WorldInfo::sendWorldOffer(peer);
									}
									else if (act == "quit") enet_peer_disconnect_later(peer, 0);
								}
							}
						} break;
						case 4:
						{
							unsigned char* tankUpdatePacket = getStructFromPacket(event.packet);
							if (tankUpdatePacket) {
								PlayerMoving* data = unpackPlayerMoving(tankUpdatePacket);

								switch (data->packetType) {
									case 0:
									{
										PlayerData* playerInfo = static_cast<PlayerData*>(peer->data);
										
										playerInfo->x = static_cast<int>(data->x);
										playerInfo->y = static_cast<int>(data->y);
										playerInfo->rotatedLeft = data->characterState & 0x10;
										PlayerState::sendData(peer, data);

										if (!playerInfo->clothesIsUpdated) {
											playerInfo->clothesIsUpdated = true;
											PlayerState::updateState(peer);
											PlayerInfo::updateCloth(peer);
										}
									} break;
									case 7:
									{
										WorldItem worldItem = world->items[data->punchX + data->punchY * world->width];
										if (getItem(worldItem.foreground).blockType == BlockType::WHITE_DOOR) {
											PlayerAction::leaveWorld(peer);
											WorldInfo::sendWorldOffer(peer);
										}
									} break;
									case 10:
									{
										// cloth
									} break;
									case 18: PlayerState::sendData(peer, data);
										break;
									case 3:
									{
										WorldTile::updateTile(Server, peer, data->punchX, data->punchY, data->plantingTree);
									} break;
								}
							}
						} break;
					}
					enet_packet_destroy(event.packet);
					break;
				} break;
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					PlayerAction::leaveWorld(peer);
					static_cast<PlayerData*>(peer->data)->inventory.items.clear();
					delete static_cast<PlayerData*>(peer->data);
					peer->data = nullptr;
				} break;
			}
		}
	}
	return 0;
}