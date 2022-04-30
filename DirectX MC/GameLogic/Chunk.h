#pragma once
#include"d3dUtil.h"
#include"TextureManager.h"
#include"MeshData.h"
#include"Collision.h"
#include"BlockRender.h"
#include <DXTrace.h>
#include"Array3D.h"
#include"Block.h"
#include"GameLoader.h"

using namespace DirectX;

class Chunk
{
	static const int chunk_width = 100;
	static const int chunk_hight = 100;

	typedef std::array<XMINT3, 6> Round;
	Round GetRound(XMINT3 pos)
	{
		return Round{
			XMINT3{pos.x + 1,pos.y,pos.z},
			XMINT3{pos.x - 1,pos.y,pos.z},
			XMINT3{pos.x,pos.y + 1,pos.z},
			XMINT3{pos.x,pos.y - 1,pos.z},
			XMINT3{pos.x,pos.y,pos.z + 1},
			XMINT3{pos.x,pos.y,pos.z - 1}
		};
	}

	Block OutBlock;//空


public:
	Chunk();

	void Generate();

	void Load(WorldDesc* desc);

	bool AddBlock(ID3D11Device* device, Ray ray, BlockType blocktype, const XMFLOAT3& playerPosition, Block*& hitBlock, bool firstPerson);

	void RemoveBlock(Ray ray, const XMFLOAT3& playerPosition);

	bool RayToLocal(Ray ray, XMINT3* hitedPos, XMINT3* abovePos, const XMINT3& playerPos, float maxDistance = 5.0f);

	XMINT3 GetLocalPos(const XMFLOAT3 pos);

	Block& GetBlock(const XMINT3& pos);

	Block& GetBlock(int x, int y, int z);

	void ReMapPos(int& x, int& y, int& z);

	bool HasBlock(const XMINT3& pos);

	bool HasBlock(int x, int y, int z);

	Transform& GetTransform();

	const Transform& GetTransform() const;

	void UpdateAll();

	void UpdatePos(XMINT3 pos);

	void SetData(BlockRender* blockbase);

	void SaveData(WorldDesc* desc);

	XMINT3 ClunkBox();
private:
	Array3D<Block, chunk_width, chunk_hight>m_chunkData;

	Transform m_Transform = {};										// 物体变换
};


