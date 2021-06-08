#pragma once
#include "Server.hpp"

#include <vector>
#include <string>
#include <fstream>

enum class ClothType {
	HAIR, SHIRT, PANTS, FEET, FACE, HAND, BACK, MASK, NECK, ANCES, NONE
};
enum class BlockType {
	NONE, FOREGROUND, BACKGROUND, SEED, PAINT_BLOCK, BEDROCK, WHITE_DOOR, SIGN,
	DOOR, CLOTHING, FIST, CONSUMMABLE, CHECKPOINT, GATEAWAY, LOCK, WEATHER_MACHINE,
	JAMMER, GEM, BOARD, WORLD_LOCK
};
enum class BlockProperty {
	UNTRADEABLE, WRENCHABLE, MULTIFACING
};

struct ItemDefinition {
	int id, dropChance, growTime, rarity;
	std::string name, description = "No description provided for this item.";
	unsigned short breakHits;
	BlockType blockType;
	ClothType clothType;
	std::vector<BlockProperty> blockProperties;
};

extern std::vector<ItemDefinition> itemDefs;

void buildItemDatabase();

inline ItemDefinition getItem(int id) {
	if (id < itemDefs.size() && id > -1) return itemDefs[id];
	return itemDefs[0];
}
inline void buildItemDescription() {
	std::ifstream ifs("config/description.txt");
	for (std::string line; std::getline(ifs, line);) {
		if ((line[0] == '/' && line[1] == '/') || line.empty()) continue;

		std::vector<std::string> info = GameInput::split("|", line);
		int itemId = std::stoi(info[0]);

		if (itemId + 1 < itemDefs.size()) {
			itemDefs[itemId].description = info[1];
			
			if ((itemId % 2) == 0)
				itemDefs[itemId + 1].description = "`$This seed grows `$" + getItem(itemId).name + "`o.";
		}
	}
}