#include "Chunk.h"
#include <WorldGenerator.h>

Chunk::Chunk() :m_chunkData(Block())
{
	WorldGenerator::GetInstance()->initMapVec();
	WorldGenerator::GetInstance()->CreateCubeMap();
	WorldGenerator::GetInstance()->CreateTreeMap();
	for (UINT y = 0; y < 100; y++)
	{
		for (UINT x = 0; x < chunk_width; x++)
		{
			for (UINT z = 0; z < chunk_width; z++)
			{
				if (WorldGenerator::GetInstance()->mapData[x][y][z] != BlockType::Null)
					m_chunkData.get(x, y, z).type = WorldGenerator::GetInstance()->mapData[x][y][z];
				if (WorldGenerator::GetInstance()->leafData[x][y][z] != BlockType::Null)
					m_chunkData.get(x, y, z).type = WorldGenerator::GetInstance()->leafData[x][y][z];


			}
		}
	}
}

void Chunk::Generate()
{
	WorldGenerator::GetInstance()->mapData.clear();
	WorldGenerator::GetInstance()->leafData.clear();
	m_chunkData.GetArray().fill(Block());
	WorldGenerator::GetInstance()->initMapVec();
	WorldGenerator::GetInstance()->CreateCubeMap();
	WorldGenerator::GetInstance()->CreateTreeMap();
	for (UINT y = 0; y < 100; y++)
	{
		for (UINT x = 0; x < chunk_width; x++)
		{
			for (UINT z = 0; z < chunk_width; z++)
			{
				m_chunkData.get(x, y, z).type = WorldGenerator::GetInstance()->mapData[x][y][z];
				if (WorldGenerator::GetInstance()->leafData[x][y][z] != BlockType::Null)
					m_chunkData.get(x, y, z).type = WorldGenerator::GetInstance()->leafData[x][y][z];
			}
		}
	}
	UpdateAll();
}


void Chunk::Load(WorldDesc* desc)
{
	UINT x = 0, y = 0, z = 0;
	auto& data = desc->worldData;

	auto iter = m_chunkData.GetArray().begin();
	for (auto& block : data)
	{
		if (iter == m_chunkData.GetArray().end()) break;
		iter->type = block;
		iter++;
	}
	UpdateAll();
}

bool Chunk::AddBlock(ID3D11Device* device, Ray ray, BlockType blocktype,
	const XMFLOAT3& playerPosition, Block*& hitBlock, bool firstPerson)
{
	XMINT3 local_pos;
	XMINT3 hit_pos;
	XMINT3 local_player_pos = GetLocalPos(playerPosition);
	if (!firstPerson)
	{
		if (RayToLocal(ray, &hit_pos, &local_pos, local_player_pos, 10.0f))
		{
			hitBlock = &m_chunkData.get(hit_pos.x, hit_pos.y, hit_pos.z);
			auto& targetblcok = GetBlock(local_pos.x, local_pos.y, local_pos.z);
			if (targetblcok.type == BlockType::ObjectOccupied)
			{
				if (local_pos.x == local_player_pos.x &&
					local_pos.y == local_player_pos.y || local_pos.y == local_player_pos.y + 1 &&
					local_pos.z == local_player_pos.z)
				{
					hitBlock = nullptr;
					/*return true;*/
				}
			}
			else if (hitBlock->type != BlockType::ObjectOccupied)
			{

				if (XMVector3Length(XMLoadSInt3(&local_pos) - XMLoadFloat3(&playerPosition)).m128_f32[0] < 5)
					targetblcok.type = blocktype;
				UpdatePos(local_pos);
				hitBlock = nullptr;
				return true;
			}
		}
	}
	else
	{
		if (RayToLocal(ray, &hit_pos, &local_pos, local_player_pos, 5))
		{
			hitBlock = &m_chunkData.get(hit_pos.x, hit_pos.y, hit_pos.z);
			auto& targetblcok = GetBlock(local_pos.x, local_pos.y, local_pos.z);
			if (targetblcok.type == BlockType::ObjectOccupied)
			{
				if (local_pos.x == local_player_pos.x &&
					local_pos.y == local_player_pos.y|| local_pos.y == local_player_pos.y+1 &&
					local_pos.z == local_player_pos.z)
				{
					hitBlock = nullptr;
					/*return true;*/
				}
			}
			else if (hitBlock->type != BlockType::ObjectOccupied)
			{
				targetblcok.type = blocktype;
				UpdatePos(local_pos);
				hitBlock = nullptr;
				return true;
			}
		}
	}
	return false;
}

void Chunk::RemoveBlock(Ray ray, const XMFLOAT3& playerPosition)
{
	XMINT3 local_pos;
	XMINT3 local_player_pos = GetLocalPos(playerPosition);

	if (RayToLocal(ray, &local_pos, nullptr, local_player_pos))
	{
		GetBlock(local_pos.x, local_pos.y, local_pos.z).type = BlockType::Null;
		UpdatePos(local_pos);
	}
}

bool Chunk::RayToLocal(Ray ray, XMINT3* hitedPos, XMINT3* abovePos,
	const XMINT3& playerPos, float maxDistance)
{
	XMMATRIX to_local = this->GetTransform().GetWorldToLocalMatrixXM();
	auto pos = XMLoadFloat3(&ray.origin);
	XMStoreFloat3(&ray.origin, XMVector3Transform(pos, to_local));
	pos = XMLoadFloat3(&ray.direction);
	XMStoreFloat3(&ray.direction, XMVector3Normalize(XMVector3Transform(pos, to_local)));

	XMINT3 origin_pos =
	{ static_cast<int32_t>(ray.origin.x + 0.5),
		static_cast<int32_t>(ray.origin.y + 0.5),
		static_cast<int32_t>(ray.origin.z + 0.5) };
	if (ray.origin.x < 0)origin_pos.x -= 1;
	if (ray.origin.y < 0)origin_pos.y -= 1;
	if (ray.origin.z < 0)origin_pos.z -= 1;
	BoundingBox central_box = BoundingBox(XMFLOAT3(origin_pos.x, origin_pos.y, origin_pos.z), XMFLOAT3(0.5, 0.5, 0.5));
	std::array<BoundingBox, 6> boxs;
	BoundingBox* getrid = nullptr;
	XMINT3 last_pos = origin_pos;
	//检测循环

	for (bool hitany = true; hitany;)
	{
		hitany = false;
		boxs.fill(central_box);
		boxs[0].Center.x += 1;
		boxs[1].Center.x -= 1;
		boxs[2].Center.y += 1;
		boxs[3].Center.y -= 1;
		boxs[4].Center.z += 1;
		boxs[5].Center.z -= 1;
		for (auto& b : boxs)
		{
			if (getrid == &b) continue;
			if (ray.Hit(b, nullptr, maxDistance))
			{
				central_box = b;
				XMINT3 hitpos;
				hitpos.x = b.Center.x;
				hitpos.y = b.Center.y;
				hitpos.z = b.Center.z;
				auto& hitblock = GetBlock(hitpos);
				if (hitblock.type == BlockType::Null ||
					hitblock.type == BlockType::ObjectOccupied &&
					hitpos.x == playerPos.x &&
					(hitpos.y == playerPos.y || hitpos.y == playerPos.y + 1) &&
					hitpos.z == playerPos.z)
				{
					XMINT3 dir{
						hitpos.x - last_pos.x,
						hitpos.y - last_pos.y,
						hitpos.z - last_pos.z };
					if (dir.x == 1)getrid = &boxs[1];
					if (dir.x == -1)getrid = &boxs[0];
					if (dir.y == 1)getrid = &boxs[3];
					if (dir.y == -1)getrid = &boxs[2];
					if (dir.z == 1)getrid = &boxs[5];
					if (dir.z == -1)getrid = &boxs[4];
					last_pos = hitpos;
					hitany = true;
					break;
				}
				else
				{
					if (hitedPos)
						*hitedPos = hitpos;
					if (abovePos)
						*abovePos = last_pos;
					return true;
				}
			}
		}
	}
	//超出距离/没有物体
	return false;
}

XMINT3 Chunk::GetLocalPos(const XMFLOAT3 pos)
{
	XMINT3 local_pos =
	{ static_cast<int32_t>(pos.x + 0.5),
		static_cast<int32_t>(pos.y + 0.5),
		static_cast<int32_t>(pos.z + 0.5) };
	if (pos.x < 0)local_pos.x -= 1;
	if (pos.y < 0)local_pos.y -= 1;
	if (pos.z < 0)local_pos.z -= 1;
	return local_pos;
}

Block& Chunk::GetBlock(const XMINT3& pos)
{
	return GetBlock(pos.x, pos.y, pos.z);
}

Block& Chunk::GetBlock(int x, int y, int z)
{
	//if (x < 0 || y < 0 || z < 0
	//	|| x >= chunk_width || y >= chunk_hight || z >= chunk_width)
	//{
	//	return OutBlock;
	//}
	if (x < 0) x += chunk_width;
	if (x >= chunk_width) x -= chunk_width;
	if (z < 0) z += chunk_width;
	if (z >= chunk_width) z -= chunk_width;
	if (y < 0 || y >= chunk_hight)return OutBlock;;
	return m_chunkData.get(x, y, z);
}

void Chunk::ReMapPos(int& x, int& y, int& z)
{
	if (x < 0) x += chunk_width;
	if (x >= chunk_width) x -= chunk_width;
	if (z < 0) z += chunk_width;
	if (z >= chunk_width) z -= chunk_width;
	if (y < 0 || y >= chunk_hight);
}

bool Chunk::HasBlock(const XMINT3& pos)
{
	auto& block = GetBlock(pos.x, pos.y, pos.z);
	if (block.type != BlockType::Null && block.type != BlockType::ObjectOccupied)
		return true;
	else
		return false;
}

bool Chunk::HasBlock(int x, int y, int z)
{
	auto& block = GetBlock(x, y, z);
	if (block.type != BlockType::Null && block.type != BlockType::ObjectOccupied)
		return true;
	else
		return false;
}

Transform& Chunk::GetTransform()
{
	return m_Transform;
}

const Transform& Chunk::GetTransform() const
{
	return m_Transform;
}

void Chunk::UpdateAll()
{
	for (int32_t y = 0; y < chunk_hight; y++)
	{
		for (int32_t x = 0; x < chunk_width; x++)
		{
			for (int32_t z = 0; z < chunk_width; z++)
			{
				XMINT3 pos = XMINT3{ x,y,z };
				auto& block = GetBlock(pos);
				if (pos.y == 0) break;
				if (block.type == BlockType::Null || block.type == BlockType::ObjectOccupied)
				{
					continue;
				}
				else
				{
					Round r = GetRound(pos);
					for (auto& p : r)
					{
						auto& b = GetBlock(p);
						if (b.type == BlockType::Null || b.type == BlockType::ObjectOccupied)
						{
							block.draw = true;
							break;
						}
					}
				}

			}
		}
	}
}

void Chunk::UpdatePos(XMINT3 pos)
{
	auto& block = GetBlock(pos);
	Round r = GetRound(pos);
	if (block.type == BlockType::Null || block.type == BlockType::ObjectOccupied)
	{
		block.draw = false;
		for (auto& p : r)
		{
			auto& b = GetBlock(p);
			if (b.type != BlockType::Null && b.type != BlockType::ObjectOccupied)
			{
				if (!b.draw)
				{
					b.draw = true;
				}
			}
		}
	}
	else
	{
		block.draw = true;
		for (auto& p : r)
		{
			auto& b = GetBlock(p);
			if (b.type != BlockType::Null && b.type != BlockType::ObjectOccupied)
			{
				b.draw = false;
				Round rr = GetRound(p);
				for (auto& pp : rr)
				{
					auto& bb = GetBlock(pp);
					if (bb.type == BlockType::Null || bb.type == BlockType::ObjectOccupied)
					{
						b.draw = true;
						break;
					}
				}
			}
		}
	}



}

void Chunk::SetData(BlockRender* blockbase)
{
	auto iter = m_chunkData.GetArray().data();
	auto& Array = m_chunkData.GetArray();
	int range = blockbase->m_RenderRange;
	auto& center = blockbase->m_RenderCenter;
	float dir_x;
	float dir_z;
	float dir_y;
	if (blockbase->m_clipSpace)
	{
		dir_x = blockbase->direction.x;
		dir_z = blockbase->direction.z;
		dir_y = blockbase->direction.y;
		float lenth = 1 - dir_y * dir_y;
		center.x -= dir_x * 3 / lenth;
		center.z -= dir_z * 3 / lenth;
		range += 3 * lenth;
	}
	int x0 = center.x - range;
	int x1 = center.x + range;
	int z0 = center.z - range;
	int z1 = center.z + range;
	Block* p;
	int xm, ym, zm;

	for (int32_t x = x0; x < x1; x++)
	{
		int32_t z{};
		int32_t z2{};//上限
		if (blockbase->m_clipSpace)
		{
			static float k0;
			static float k1;
			static float k3;
			static float k4;
			static float dx;
			k0 = dir_z / dir_x;
			k1 = 1.3;
			k3 = (k0 + k1) / (1 - k0 * k1);
			k4 = (k0 - k1) / (1 + k0 * k1);
			dx = x - center.x;
			if (k3 < 0 || k4>0)
			{
				if (dir_z > 0)
				{
					if (dx < 0)
						z = center.z + k3 * dx;
					else
						z = center.z + k4 * dx;
					z2 = z1;
				}
				else //dir_z<0
				{
					if (dx < 0)
						z2 = center.z + k4 * dx;
					else
						z2 = center.z + k3 * dx;
					z = z0;
				}
			}
			else if (dir_x > 0 && k3 > 0)
			{
				z = center.z + k4 * dx;
				z2 = center.z + k3 * dx;
			}
			else if (k4 < 0 && dir_x < 0)
			{
				z = center.z + k3 * dx;
				z2 = center.z + k4 * dx;
			}
			if (z < z0)z = z0;
			if (z2 > z1) z2 = z1;
		}
		else
		{
			z = z0;
			z2 = z1;
		}
		for (; z < z2; z++)
		{
			for (int32_t y = 0; y < chunk_hight; y++)
			{
				xm = x;
				ym = y;
				zm = z;
				ReMapPos(xm, ym, zm);
				p = &m_chunkData.get(xm, ym, zm);
				if (p->draw)
				{
					blockbase->put(p->type, XMINT3{ x,y,z });
				}
			}
		}
	}

}

void Chunk::SaveData(WorldDesc* desc)
{
	desc->worldData.resize(chunk_hight * chunk_width * chunk_width);
	auto iter = desc->worldData.begin();
	for (auto& block : m_chunkData.GetArray())
	{
		*iter = block.type;
		iter++;
	}

}

XMINT3 Chunk::ClunkBox()
{
	return XMINT3(chunk_width, chunk_hight, chunk_width);
}
