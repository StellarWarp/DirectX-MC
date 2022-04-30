#include "Creature.h"



void Creature::movingAnimation()
{

}

void Creature::SetMoveSpeed(float newSpeed)
{
	max_speed = newSpeed;
}

XMFLOAT3& Creature::GetPosition()
{
	return CreaturePos;
}

XMFLOAT3 Creature::GetPositionWithOffset()
{
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, XMLoadFloat3(&PosOffset) + XMLoadFloat3(&CreaturePos));
	return pos;
}

void Creature::Update(float dt)
{
	jumped = false;
	if (GetWorld()->RemapPos(CreaturePos))
	{
		shouldRender = false;
	}
	else
	{
		shouldRender = true;
	}
	UpdatePhysic(dt);
	m_Transform.SetPosition(PosOffset.x + CreaturePos.x, PosOffset.y + CreaturePos.y - 0.1, PosOffset.z + CreaturePos.z);
	if (CreaturePos.y < 0)
	{
		HitPoint = 0;
		shouldRemove = true;
	}
}


void Creature::Move(const XMFLOAT3& dir, float dt)
{
	float f = 10;
	XMFLOAT3 Force;
	float forceScale = 20;
	if (!onGound)
	{
		forceScale = 5;
	}
	XMVECTOR ex_speed = XMVector3Normalize(XMLoadFloat3(&dir)) * max_speed;
	XMStoreFloat3(&Force,
		(ex_speed - XMLoadFloat3(&m_speed)) * forceScale
	);
	Force.y = 0;

	ApplyForce(Force);

	XMINT3 i_dir{};
	XMFLOAT3 ex_speed_f;
	XMStoreFloat3(&ex_speed_f, ex_speed);
	float k = max_speed * 0.5;
	if (ex_speed_f.x > k)
	{
		i_dir.x = 1;
	}
	else if (ex_speed_f.x < -k)
	{
		i_dir.x = -1;
	}
	if (ex_speed_f.z > k)
	{
		i_dir.z = 1;
	}
	else if (ex_speed_f.z < -k)
	{
		i_dir.z = -1;
	}
	if (!HasCell())
	{
		auto impl = m_pWorld->GetChunk();
		auto local_pos = impl->GetLocalPos(CreaturePos);
		if (impl->HasBlock(local_pos.x + i_dir.x, local_pos.y, local_pos.z) &&
			!impl->HasBlock(local_pos.x + i_dir.x, local_pos.y + 1, local_pos.z))
		{
			if (!jumped)
			{
				Jump(dt);
			}
		}
		if (impl->HasBlock(local_pos.x, local_pos.y, local_pos.z + i_dir.z) &&
			!impl->HasBlock(local_pos.x, local_pos.y + 1, local_pos.z + i_dir.z))
		{
			if (!jumped)
			{
				Jump(dt);
			}
		}
		if (impl->HasBlock(local_pos.x + i_dir.x, local_pos.y, local_pos.z + i_dir.z) &&
			!impl->HasBlock(local_pos.x + i_dir.x, local_pos.y + 1, local_pos.z + i_dir.z))
		{
			if (!jumped)
			{
				Jump(dt);
			}
		}
	}

}
void Creature::Jump(float dt)
{
	if (onGound&&!jumped)
	{
		float jumpMomentum = 4.5;
		XMFLOAT3 Force(0, jumpMomentum/dt, 0);
		jumped = true;
		ApplyForce(Force);
	}
}

void Creature::Hit(int damge)
{
	HitPoint -= damge;
	if (HitPoint <= 0)
	{
		shouldRemove = true;
	}
}

void Creature::Attack(Creature* target, float dt)
{
	target->Hit(damage);
	XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&target->CreaturePos) - XMLoadFloat3(&CreaturePos));
	XMFLOAT3 Force{};
	float hitMomentum = damage*10;
	XMStoreFloat3(&Force, dir * hitMomentum /dt);
	Force.y = hitMomentum/5 / dt;
	target->ApplyForce(Force);
}

void Creature::EnhanceDamage(float percentage)
{
	damage = damage * percentage;
}

void Creature::ResetDamage()
{
	damage = basic_damage;
}

void Creature::AddDamage(int add)
{
	damage += add;
}
