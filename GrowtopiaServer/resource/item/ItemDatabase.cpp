#include "../../Item.hpp"
#include "../../Server.hpp"

using namespace std;

void buildItemDatabase() {
	{
		string filePath = "items.dat";
		ifstream ifs(filePath, ios::binary | ios::ate);
		int size = static_cast<int>(ifs.tellg());
		ItemsDatSize = size;
		char* data = new char[size];
		ifs.seekg(0, ios::beg);

		if (ifs.read(static_cast<char*>(data), size)) {
			ItemsDatData = new unsigned char[60 + size];

			int messageType = 0x4, packetType = 0x10, netId = -1, charState = 0x8;
			memset(ItemsDatData, 0, 60);

			memcpy(ItemsDatData, &messageType, 4);
			memcpy(ItemsDatData + 4, &packetType, 4);
			memcpy(ItemsDatData + 8, &netId, 4);
			memcpy(ItemsDatData + 16, &charState, 4);
			memcpy(ItemsDatData + 56, &size, 4);
			ifs.seekg(0, ios::beg);

			if (ifs.read(reinterpret_cast<char*>(ItemsDatData + 60), size)) {
				unsigned char* pData;
				int pSize = 0;
				char fileName[] = "items.dat";

				pSize = static_cast<int>(fileSize(fileName));
				pData = getFileAttribute(filePath, &pSize);
				ItemsDatHash = hashString(static_cast<unsigned char*>(pData), pSize);
				ifs.close();
			}
		}
	}

	ifstream ifs("config/coredata.txt");
	for (string line; getline(ifs, line);) {
		vector<string> info = GameInput::split("|", line);
		ItemDefinition item;
		item.id = stoi(info[0]);
		item.name = info[1];
		item.rarity = stoi(info[2]);

		for (string properties : GameInput::split(",", info[3])) {
			if (properties == "MultiFacing") item.blockProperties.push_back(BlockProperty::MULTIFACING);
		}
		string type = info[4];
		
		if (type == "Foreground_Block") item.blockType = BlockType::FOREGROUND;

		else if (type == "Background_Block") item.blockType = BlockType::BACKGROUND;

		else if (type == "Main_Door") item.blockType = BlockType::WHITE_DOOR;

		else if (type == "Lock") {
			if (item.id != 202 && item.id != 204 && item.id != 206) item.blockType = BlockType::WORLD_LOCK;
		}
		else if (type == "Sign") item.blockType = BlockType::SIGN;

		else if (type == "Clothing") {
			item.blockType = BlockType::CLOTHING;
			string cloth = info[9];

			if (cloth == "None") {
				item.clothType = ClothType::NONE;
			}
			else if (cloth == "Hat") {
				item.clothType = ClothType::HAIR;
			}
			else if (cloth == "Shirt") {
				item.clothType = ClothType::SHIRT;
			}
			else if (cloth == "Pants") {
				item.clothType = ClothType::PANTS;
			}
			else if (cloth == "Feet") {
				item.clothType = ClothType::FEET;
			}
			else if (cloth == "Face") {
				item.clothType = ClothType::FACE;
			}
			else if (cloth == "Hand") {
				item.clothType = ClothType::HAND;
			}
			else if (cloth == "Back") {
				item.clothType = ClothType::BACK;
			}
			else if (cloth == "Hair") {
				item.clothType = ClothType::MASK;
			}
			else if (cloth == "Chest") {
				item.clothType = ClothType::NECK;
			}
			else if (type == "Artifact") {
				item.clothType = ClothType::ANCES;
			}
			else {
				item.clothType = ClothType::NONE;
			}
		}
		else item.blockType = BlockType::NONE;

		itemDefs.push_back(item);
	}
	buildItemDescription();
}