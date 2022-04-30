#pragma once
#include"TextureManager.h"

enum class BlockType
{
	Null,				// 空
	MapLand,			// 土块
	MapSand,			// 沙子
	MapStone,			// 石头
	MapRedBrick,		// 红砖
	MapWood,			// 木头

	MapGrassland,		// 草地
	MapTree,			// 树木
	MapSnow,			// 雪地

	MapLeaveGreen,		// 绿树叶
	MapLeaveRed,		// 红树叶
	MapLeaveYellow,		// 黄树叶
	MapLeaveWhite,		// 白树叶

	MapGlass,     //玻璃

	ObjectOccupied, //有物体
};

struct Block
{
	BlockType type;
	//UINT64 id;
	bool draw;
	//属性
	Block()
		:type(BlockType::Null),
		draw(false)
	{}
	std::vector<UINT>* objID = nullptr;
	void AddObj(UINT id)
	{
		if (objID)
		{
			objID->push_back(id);
		}
		else
		{
			objID = new std::vector<UINT>;
			objID->push_back(id);
			if (type == BlockType::Null)
				type = BlockType::ObjectOccupied;
		}
	}
	void RemoveObj(UINT id)
	{
		if (objID)
		{
			auto iter = std::find(objID->begin(), objID->end(), id);
			objID->erase(iter);
			if (objID->size() == 0)
			{
				delete objID;
				objID = nullptr;
				if (type == BlockType::ObjectOccupied)
					type = BlockType::Null;
			}
		}
	}
};
