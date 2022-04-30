#pragma once
#include"Chunk.h"
#include"BlockRender.h"

class World
{
public:
	World()
	{}

	XMINT3 RenderCenter;


	void init(ID3D11Device* device);

	void GenerateWorld();

	void Load(WorldDesc* desc);
	void Save(WorldDesc* desc);

	void OperateBlock(ID3D11Device* device, Ray ray, BlockType blocktype, Block*& target, const XMFLOAT3& playerPosition, bool firstPerson);

	void RemoveBlock(Ray ray, const XMFLOAT3& playerPosition);
	bool RemapPos(XMFLOAT3& pos);
	void SetRenderCenter(XMFLOAT3& PlayerPos, const XMFLOAT3& center, const XMFLOAT3& direction);

	XMINT3 GetBlockPos(const XMFLOAT3& pos);
	XMINT3 GetHighPoint(int x, int z)
	{
		for (int i = m_Chunk.ClunkBox().y - 1; i > 0; i--)
		{
			if (m_Chunk.HasBlock(x, i, z))
			{
				return XMINT3(x, i - 1, z);
			}
		}
		return XMINT3(x, m_Chunk.ClunkBox().y - 1, z);
	}
	void Draw(ID3D11DeviceContext* device, IEffect* effect, bool clip);
	Chunk* GetChunk();

	void SetDebugObjectName(const std::string& name);
private:
	BlockRender m_BlockRender{};
	Chunk m_Chunk{};
};

