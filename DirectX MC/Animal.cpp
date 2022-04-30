#include "Animal.h"

Model* Animal::Animal_Model = nullptr;
int Animal::num = 0;
void Animal::init_Animal(ID3D11Device* device, World* world)
{
	if (!Animal_Model)
	{
		//model
		ObjReader objReader;
		objReader.Read(nullptr, L"..\\Model\\Creeper.obj");
		Material material;
		material.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.3f);
		material.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		material.specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
		material.reflect = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

		auto iter = objReader.objParts.begin();
		while (iter != objReader.objParts.end())
		{
			if (iter->vertices.size() == 0)
			{
				iter = objReader.objParts.erase(iter);
			}
			else
				iter++;
		}
		for (auto& part : objReader.objParts)
		{
			part.material = material;
			part.texStrDiffuse = L"..\\Model\\Creeper.png";
		}
		SetModel(Model(device, objReader));
		GetTransform().SetScale(0.14f, 0.14f, 0.14f);
		m_Model.modelParts[6].visible = false;
		Animal_Model = new Model(m_Model);
	}
	else
	{
		m_Model = *Animal_Model;
		GetTransform().SetScale(0.14f, 0.14f, 0.14f);
	}
	init_Animal(world);
}

void Animal::init_Animal(World* world)
{
	if (!Animal_Model)
	{
		throw("model init!");
	}
	init_Physic(world);
	num++;
	SetMoveSpeed(1);
}

void Animal::Load(WorldDesc::CreatureDesc desc, World* world)
{
	init_Animal(world);
	GetTransform().SetScale(0.14f, 0.14f, 0.14f);
	HitPoint = desc.HP;
	CreaturePos = desc.pos;
}

void Animal::Save(WorldDesc::CreatureDesc* desc)
{
	desc->HP = HitPoint;
	desc->pos = CreaturePos;
}

void Animal::Wander(float dt)
{
	float& time = animval.wander_T;
	float& t = animval.wander_t;
	float& dir1 = animval.wander_dir1;
	float& dir2 = animval.wander_dir2;
	t += dt;
	if (t > time)
	{
		t = 0;
		dir1 = rand() % 100-50;
		dir2 = rand() % 100-50;
	}
	MoveDirection.x = dir1;
	MoveDirection.z = dir2;
	Move(MoveDirection, dt);
	is_walk = true;
}

void Animal::Process(float dt)
{
	Update(dt);
	Wander(dt);
	GetTransform().SetRotation(0, -XM_PI, 0);
	UpdateAnimation(dt);

}

void Animal::WalkAnimation(float legAngle)
{
	//后
	GetPartTransform(2).SetRotation(0, 0, 0);
	GetPartTransform(2).SetPosition(0, 0, 0);
	GetPartTransform(2).RotateAround(XMFLOAT3(0, 2.7, -1),
		XMFLOAT3(1, 0, 0), legAngle);
	//后
	GetPartTransform(3).SetRotation(0, 0, 0);
	GetPartTransform(3).SetPosition(0, 0, 0);
	GetPartTransform(3).RotateAround(XMFLOAT3(0, 2.7, 1),
		XMFLOAT3(1, 0, 0), -legAngle);
	//前
	GetPartTransform(4).SetRotation(0, 0, 0);
	GetPartTransform(4).SetPosition(0, 0, 0);
	GetPartTransform(4).RotateAround(XMFLOAT3(0, 2.7, -1),
		XMFLOAT3(1, 0, 0), -legAngle);
	//后
	GetPartTransform(5).SetRotation(0, 0, 0);
	GetPartTransform(5).SetPosition(0, 0, 0);
	GetPartTransform(5).RotateAround(XMFLOAT3(0, 2.7, 1),
		XMFLOAT3(1, 0, 0), legAngle);

	PosOffset.y = -abs(sin(legAngle) * 0.1);
}

void Animal::UpdateAnimation(float dt)
{
	float& current_angle = animval.current_angle;
	float body_turn_angle = atan2f(MoveDirection.x , MoveDirection.z);

	//body turn
	{
		current_angle += (body_turn_angle - current_angle) * 5 * dt;

		auto roate = GetTransform().GetRotation();
		roate.y += current_angle;
		//GetPartTransform(0).RotateAround(XMFLOAT3(0, 7, 0),
		//	XMFLOAT3(0, 1, 0), -current_angle);
		GetTransform().SetRotation(roate);
	}
	if (is_walk)
	{
		static float time = 1;
		static float omega = XM_2PI / time;
		float& t = animval.t1;
		bool& priod_control = animval.priod_control;
		t += dt;
		if (t > time / 2)
		{
			t = 0;
			is_walk = false;
			priod_control = !priod_control;
		};
		float angle;
		if (priod_control)
		{
			angle = std::sin((t + 0.5) * omega) * XM_PI / 8;
		}
		else
		{
			angle = std::sin(t * omega) * XM_PI / 12;
		}
		WalkAnimation(angle);
	}

	UpdateHp(body_turn_angle);
}

void Animal::UpdateHp(float bodyRoateAngle)
{
	float& show_percentage = animval.ph_show_percentage;
	float& value = animval.hp_value;
	if (PlayerPos)
	{
		XMVECTOR dir = XMLoadFloat3(PlayerPos) - XMLoadFloat3(&CreaturePos);
		float distance = XMVector3Length(dir).m128_f32[0];
		if (distance > 6)
		{
			if (show_percentage > 0)
			{
				show_percentage -= 0.05;
			}
			if (show_percentage < 0)
			{
				PlayerPos = nullptr;
				m_Model.modelParts[6].visible = false;
				return;
			}
		}
		else
		{
			if (show_percentage < 1)
			{
				show_percentage += 0.05;
			}
		}
		XMFLOAT3 dir_f;
		XMStoreFloat3(&dir_f, dir);
		float angle = atan(dir_f.z / dir_f.x);
		if (dir_f.x < 0)angle += XM_PI;

		float hp_persentage = (float)HitPoint / (float)MaxHitPoint;
		if (value != hp_persentage)
		{
			value -= (value - hp_persentage) * 0.1;
		}

		auto& HpBar = m_Model.modelParts[6].transform;
		HpBar.SetScale(show_percentage * value, 1, 1);
		HpBar.SetRotation(0, -angle - bodyRoateAngle + XM_PIDIV2, 0);
	}

}

bool Animal::InRange(XMFLOAT3& Player_pos)
{
	if (PlayerPos) return true;
	PlayerPos = &Player_pos;
	m_Model.modelParts[6].visible = true;
	UpdateHp(0);
	return true;
}

void Animal::HpModel(bool on)
{
	if (on && PlayerPos)
	{
		m_Model.modelParts[6].visible = true;
	}
	else
	{
		m_Model.modelParts[6].visible = false;
	}
}

int Animal::GetNumber()
{
	return num;
}
