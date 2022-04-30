#include "World.h"
#include <d3dApp.h>

void World::init(ID3D11Device* device)
{
	m_Chunk.UpdateAll();
	m_BlockRender.init(device);
}

void World::GenerateWorld()
{
	m_Chunk.Generate();
}
void World::Load(WorldDesc* desc)
{
	m_Chunk.Load(desc);
	m_Chunk.UpdateAll();
}

void World::Save(WorldDesc* desc)
{
	m_Chunk.SaveData(desc);
}

//是否有目标

void World::OperateBlock(ID3D11Device* device, Ray ray, BlockType blocktype,
	Block*& target, const XMFLOAT3& playerPosition, bool firstPerson)
{
	m_Chunk.AddBlock(device, ray, blocktype, playerPosition, target, firstPerson);
}

//}

void World::RemoveBlock(Ray ray, const XMFLOAT3& playerPosition)
{
	m_Chunk.RemoveBlock(ray, playerPosition);
}

//if outof distance
bool World::RemapPos(XMFLOAT3& pos)
{
	auto range = m_Chunk.ClunkBox();
	auto& center = m_BlockRender.m_RenderCenter;
	auto render_range = m_BlockRender.m_RenderRange;
	int x0 = center.x - range.x / 2;
	int x1 = center.x + range.x / 2;
	int z0 = center.z - range.z / 2;
	int z1 = center.z + range.z / 2;
	while (pos.x > x1)
	{
		pos.x -= range.x;
	}
	while (pos.x < x0)
	{
		pos.x += range.x;
	}
	while (pos.z > z1)
	{
		pos.z -= range.z;
	}
	while (pos.z < z0)
	{
		pos.z += range.z;
	}
	if (pos.x< center.x - render_range ||
		pos.x > center.x + render_range ||
		pos.y< center.y - render_range ||
		pos.y > center.y + render_range)
	{
		return true;
	}
	return false;
}
void World::SetRenderCenter(XMFLOAT3& PlayerPos, const XMFLOAT3& center, const XMFLOAT3& direction)
{
	auto range = m_Chunk.ClunkBox();
	auto PlayerPos_i = GetBlockPos(PlayerPos);
	auto center_i = GetBlockPos(center);
	if (center_i.x > range.x)
	{
		center_i.x -= range.x;
		PlayerPos.x -= range.x;
	}
	if (center_i.x < 0)
	{
		center_i.x += range.x;
		PlayerPos.x += range.x;
	}

	if (center_i.z > range.z)
	{
		center_i.z -= range.z;
		PlayerPos.z -= range.z;
	}
	if (center_i.z < 0)
	{
		center_i.z += range.z;
		PlayerPos.z += range.z;
	}
	m_BlockRender.m_RenderCenter = center_i;
	m_BlockRender.direction = direction;
}

XMINT3 World::GetBlockPos(const XMFLOAT3& pos)
{
	return m_Chunk.GetLocalPos(pos);
}

void World::Draw(ID3D11DeviceContext* device, IEffect* effect, bool clip)
{
	m_BlockRender.reset();
	m_BlockRender.m_clipSpace = clip;
	m_Chunk.SetData(&m_BlockRender);
	m_BlockRender.Draw(device, effect);
}

Chunk* World::GetChunk()
{
	return &m_Chunk;
}

void World::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)

	m_BlockRender.SetDebugObjectName(name);

#endif
}
