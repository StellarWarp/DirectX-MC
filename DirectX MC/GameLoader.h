#pragma once
#include <DirectXMath.h>
#include<fstream>
#include<string>
#include<vector>
#include"Block.h"
#include"InventoryItem.h"


struct WorldDesc
{
	struct CreatureDesc
	{
		DirectX::XMFLOAT3 pos;
		int HP;
	};
	struct PlayerDesc
	{
		DirectX::XMFLOAT3 pos;
		int HP;
		//type num
		InventoryItem inventory_item[10];
		UINT16 inventory_num[10];
		
	};
	std::vector<BlockType> worldData;
	PlayerDesc playerData;
	std::vector<CreatureDesc> creatureData;
	float time;
};

class GameLoader
{
public:
	std::string foder = "GameData/";
	WorldDesc desc;

	static GameLoader* GetInstance();

	WorldDesc* GetDesc();
	void SaveGame(std::string file);
	void Clear();
	bool LoadGame(std::string file);
};

