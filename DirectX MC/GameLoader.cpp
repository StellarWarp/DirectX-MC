#include "GameLoader.h"

static GameLoader g_Instance;

GameLoader* GameLoader::GetInstance()
{
	return &g_Instance;
}

WorldDesc* GameLoader::GetDesc()
{
	return &desc;
}

void GameLoader::SaveGame(std::string file)
{
	std::ofstream blockfile;
	std::ofstream playerfile;
	std::ofstream creaturefile;
	blockfile.open(foder + file + "_block.bin", std::ios::binary);
	playerfile.open(foder + file + "_player.bin", std::ios::binary);
	creaturefile.open(foder + file + "_creature.bin", std::ios::binary);
	for (auto& data : desc.worldData)
	{
		blockfile.write((char*)&data, sizeof(BlockType));
	}
	for (auto& data : desc.creatureData)
	{
		creaturefile.write((char*)&data, sizeof(WorldDesc::CreatureDesc));
	}
	playerfile.write((char*)&desc.playerData, sizeof(WorldDesc::PlayerDesc));

}

void GameLoader::Clear()
{
	desc.worldData.clear();
	desc.creatureData.clear();
}
bool GameLoader::LoadGame(std::string file)
{
	std::ifstream blockfile;
	std::ifstream playerfile;
	std::ifstream creaturefile;
	blockfile.open(foder + file + "_block.bin", std::ios::binary);
	playerfile.open(foder + file + "_player.bin", std::ios::binary);
	creaturefile.open(foder + file + "_creature.bin", std::ios::binary);
	if (!blockfile.is_open() || !playerfile.is_open() || !creaturefile.is_open())
	{
		return false;
	}
	while(!blockfile.eof())
	{
		BlockType data{};
		blockfile.read((char*)&data, sizeof(BlockType));
		desc.worldData.push_back(data);
	}
	while (!creaturefile.eof())
	{
		WorldDesc::CreatureDesc data{};
		creaturefile.read((char*)&data, sizeof(WorldDesc::CreatureDesc));
		desc.creatureData.push_back(data);
	}
	playerfile.read((char*)&desc.playerData, sizeof(WorldDesc::PlayerDesc));

	return true;
}
