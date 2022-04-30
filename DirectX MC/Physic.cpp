#include "Physic.h"

UINT Physic::objects_count;
std::unordered_map<UINT, Physic*> Physic::Objects;

Physic::Physic(XMFLOAT3* pos)
{
	objects_count++;
	ID = objects_count;
	Objects.insert(std::pair(ID, this));
	this->pos = pos;
}

void Physic::init_Physic(World* world)
{
	m_pWorld = world;
}

bool Physic::HasGound()
{
	auto impl = m_pWorld->GetChunk();
	auto local_pos = impl->GetLocalPos(*pos);
	if (impl->HasBlock(local_pos.x, local_pos.y - 1, local_pos.z))
	{
		return true;
	}
	return false;
	/*for (auto& epos : edge_pos)
	{
		auto local_pos = impl->GetLocalPos(epos);
		local_pos.y--;
		auto blockType = impl->GetBlock(local_pos).type;
		if (blockType != BlockType::Null && blockType != BlockType::ObjectOccupied)
		{
			return true;
		}
	}
	return false;*/
}
void Physic::unuse()
{
}
bool Physic::HasCell()
{
	auto impl = m_pWorld->GetChunk();
	auto local_pos = impl->GetLocalPos(*pos);
	if (impl->HasBlock(local_pos.x, local_pos.y + 2, local_pos.z))
	{
		return true;
	}
	return false;
	/*for (auto& epos : edge_pos)
	{
		auto local_pos = impl->GetLocalPos(epos);
		local_pos.y += 2;
		auto blockType = impl->GetBlock(local_pos).type;
		if (blockType != BlockType::Null && blockType != BlockType::ObjectOccupied)
		{
			return true;
		}
	}
	return false;*/
}

void Physic::NormalProcess(float dt)
{
	//Gravity
	if (!onGound)
	{
		float g = 15;
		XMStoreFloat3(&m_force, XMLoadFloat3(&m_force) + XMVECTOR{ 0,-g ,0,0 });
	}

	XMStoreFloat3(&m_speed, XMLoadFloat3(&m_force) * dt / m_mass + XMLoadFloat3(&m_speed));
	XMStoreFloat3(pos, XMLoadFloat3(&m_speed) * dt + XMLoadFloat3(pos));



	ZeroMemory(&m_force, sizeof(XMFLOAT3));
}

void Physic::Collision(float dt)
{
	auto impl = m_pWorld->GetChunk();
	auto local_pos = impl->GetLocalPos(*pos);
	auto& block = impl->GetBlock(local_pos);
	//更新位置
	if (world_pos.x != local_pos.x ||
		world_pos.y != local_pos.y ||
		world_pos.z != local_pos.z)
	{
		auto& block1 = impl->GetBlock(world_pos);
		world_pos.y += 1;
		auto& block2 = impl->GetBlock(world_pos);
		block1.RemoveObj(ID);
		block2.RemoveObj(ID);

		XMINT3 n_pos = local_pos;
		n_pos.y += 1;
		auto& n_block1 = impl->GetBlock(local_pos);
		auto& n_block2 = impl->GetBlock(n_pos);
		n_block1.AddObj(ID);
		n_block2.AddObj(ID);
		world_pos = local_pos;
	}
	//作用力
	if (block.objID && block.objID->size() > 1)
	{
		for (auto id : *block.objID)
		{
			if (id == ID) continue;
			XMVECTOR dir = XMLoadFloat3(pos) - XMLoadFloat3(Objects[id]->pos);
			float distance = XMVector3Length(dir).m128_f32[0];
			if (distance < radius * 2)
			{
				XMFLOAT3 force;
				XMStoreFloat3(&force, dir / distance * (radius * 2 - distance)/dt);
				ApplyForce(force);
			}
		}
	}

}

void Physic::ApplyForce(const XMFLOAT3& _force)
{
	XMStoreFloat3(&m_force, XMLoadFloat3(&_force) + XMLoadFloat3(&m_force));
}
void Physic::UpdatePhysic(float dt)
{

	edge_pos[0] = XMFLOAT3(pos->x + radius, pos->y, pos->z);
	edge_pos[1] = XMFLOAT3(pos->x - radius, pos->y, pos->z);
	edge_pos[2] = XMFLOAT3(pos->x, pos->y, pos->z + radius);
	edge_pos[3] = XMFLOAT3(pos->x, pos->y, pos->z - radius);

	pos_offset = XMFLOAT3{
		pos->x - static_cast<int32_t>(pos->x + 0.5f),
		pos->y - static_cast<int32_t>(pos->y + 0.5f),
		pos->z - static_cast<int32_t>(pos->z + 0.5f) };

	auto impl = m_pWorld->GetChunk();
	auto local_pos = impl->GetLocalPos(*pos);

	float hit_factor = 0.01;
	float force_scale = std::abs(pos_offset.x) - s;
	if (force_scale < 0)
	{
		force_scale = std::abs(pos_offset.y) - s;
	}
	float blockforce = hitforce * force_scale * force_scale;

	//if (impl->HasBlock(local_pos))
	//{
	//	pos->y += pos_offset.y;
	//	if (m_speed.y < 0)
	//	{
	//		m_speed.y = -m_speed.y * 0.1;
	//	}
	//}
	if (pos_offset.y + 0.5f < 0.1 && HasGound())
	{

		m_speed.y = -m_speed.y * 0.1;
		if (m_speed.y < 0.1)
		{
			m_speed.y = 0;
		}
		/*ApplyForce(XMFLOAT3(0, blockforce, 0));*/
		onGound = true;
	}
	else
	{
		onGound = false;
	}
	if (pos_offset.y + 0.5f > 2 - hight - 0.01 && HasCell())
	{
		m_speed.y = -m_speed.y * 0.5;
		ApplyForce(XMFLOAT3(0, -blockforce, 0));
	}
	if (pos_offset.x > s)
	{
		if (impl->HasBlock(local_pos.x + 1, local_pos.y, local_pos.z) ||
			impl->HasBlock(local_pos.x + 1, local_pos.y + 1, local_pos.z))
		{
			if (m_speed.x > 0) m_speed.x = -m_speed.x * hit_factor;
			if (m_force.x > 0) m_force.x = 0;
		}
	}
	if (pos_offset.x < -s)
	{
		if (impl->HasBlock(local_pos.x - 1, local_pos.y, local_pos.z) ||
			impl->HasBlock(local_pos.x - 1, local_pos.y + 1, local_pos.z))
		{
			if (m_speed.x < 0)m_speed.x = -m_speed.x * hit_factor;
			if (m_force.x < 0) m_force.x = 0;
		}
	}
	if (pos_offset.z > s)
	{
		if (impl->HasBlock(local_pos.x, local_pos.y, local_pos.z + 1) ||
			impl->HasBlock(local_pos.x, local_pos.y + 1, local_pos.z + 1))
		{
 			if (m_speed.z > 0)m_speed.z = -m_speed.z * hit_factor;
			if (m_force.z > 0) m_force.z = 0;
		}
	}
	if (pos_offset.z < -s)
	{
		if (impl->HasBlock(local_pos.x, local_pos.y, local_pos.z - 1) ||
			impl->HasBlock(local_pos.x, local_pos.y + 1, local_pos.z - 1))
		{
			if (m_speed.z < 0)m_speed.z = -m_speed.z * hit_factor;
			if (m_force.z < 0) m_force.z = 0;
		}
	}
	if (pos_offset.x > s && pos_offset.z > s)
	{
		if (impl->HasBlock(local_pos.x + 1, local_pos.y, local_pos.z + 1) ||
			impl->HasBlock(local_pos.x + 1, local_pos.y + 1, local_pos.z + 1))
		{
			if (pos_offset.x < pos_offset.z)
			{
				if (m_speed.x > 0)m_speed.x = -m_speed.x * hit_factor;
				if (m_force.x > 0) m_force.x = 0;
			}
			else
			{
				if (m_speed.z > 0)m_speed.z = -m_speed.z * hit_factor;
				if (m_force.z > 0) m_force.z = 0;
			}
		}
	}
	if (pos_offset.x > s && pos_offset.z < -s)
	{
		if (impl->HasBlock(local_pos.x + 1, local_pos.y, local_pos.z - 1) ||
			impl->HasBlock(local_pos.x + 1, local_pos.y + 1, local_pos.z - 1))
		{
			if (pos_offset.x < -pos_offset.z)
			{
				if (m_speed.x > 0)m_speed.x = -m_speed.x * hit_factor;
				if (m_force.x > 0) m_force.x = 0;
			}
			else
			{
				if (m_speed.z < 0)m_speed.z = -m_speed.z * hit_factor;
				if (m_force.z < 0) m_force.z = 0;
			}
		}
	}
	if (pos_offset.x < -s && pos_offset.z > s)
	{
		if (impl->HasBlock(local_pos.x - 1, local_pos.y, local_pos.z + 1) ||
			impl->HasBlock(local_pos.x - 1, local_pos.y + 1, local_pos.z + 1))
		{
			if (pos_offset.x > -pos_offset.z)
			{
				if (m_speed.x < 0)m_speed.x = -m_speed.x * hit_factor;
				if (m_force.x < 0) m_force.x = 0;
			}
			else
			{
				if (m_speed.z > 0)m_speed.z = -m_speed.z * hit_factor;
				if (m_force.z > 0) m_force.z = 0;
			}
		}
	}
	if (pos_offset.x < -s && pos_offset.z < -s)
	{
		if (impl->HasBlock(local_pos.x - 1, local_pos.y, local_pos.z - 1) ||
			impl->HasBlock(local_pos.x - 1, local_pos.y + 1, local_pos.z - 1))
		{
			if (pos_offset.x > pos_offset.z)
			{
				if (m_speed.x < 0)m_speed.x = -m_speed.x * hit_factor;
				if (m_force.x < 0) m_force.x = 0;
			}
			else
			{
				if (m_speed.z < 0)m_speed.z = -m_speed.z * hit_factor;
				if (m_force.z < 0) m_force.z = 0;
			}
		}
	}

	//ImGui::Begin("Debug");
	//ImGui::Text("Position %.1f   %.1f   %.1f",
	//	pos_offset.x,
	//	pos_offset.y,
	//	pos_offset.z);
	//ImGui::End();

	Collision(dt);
	NormalProcess(dt);
}