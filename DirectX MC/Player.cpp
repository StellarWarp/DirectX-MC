#include "Player.h"
#include"GameLoader.h"
void Player::init_Player(ID3D11Device* device, World* world)
{
	ObjReader objReader;
	objReader.Read(nullptr, L"..\\Model\\character.obj");
	Material material;
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
	material.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	auto iter = objReader.objParts.begin();
	while (iter != objReader.objParts.end())
	{
		if (iter->vertices.size() == 0 || iter->texStrDiffuse == L"")
		{
			iter = objReader.objParts.erase(iter);
		}
		else
			iter++;
	}
	for (auto& part : objReader.objParts)
	{
		part.material = material;
		part.texStrDiffuse = L"..\\Model\\2.png";
	}

	SetModel(Model(device, objReader));
	GetTransform().SetScale(0.2f, 0.2f, 0.2f);

	init_Physic(world);
}

void Player::Load(WorldDesc::PlayerDesc desc)
{
	HitPoint = desc.HP;
	CreaturePos = desc.pos;
	auto itern = desc.inventory_num;
	auto iteri = desc.inventory_item;
	for (auto& s : pack.data)
	{
		s.item = *iteri;
		s.num = *itern;
		iteri++;
		itern++;

	}
}

void Player::Save(WorldDesc::PlayerDesc* desc)
{
	int i = 0;
	for (auto& s : pack.data)
	{
		desc->inventory_item[i] = (s.item);
		desc->inventory_num[i] = (s.num);
		i++;
		if (i >= 10)break;
	}
	desc->HP = HitPoint;
	desc->pos = CreaturePos;
}

//储物
void Player::SelectSort(int i)
{
	currentsort = itemsort[i];
}
InventoryItem Player::GetItem()
{
	return currentsort->item;
}
BlockType Player::GetBlockType()
{
	if (currentsort)
		return static_cast<BlockType>(currentsort->item);//unsafe
	else
		return BlockType::Null;
}
void Player::UseItem()
{
	currentsort->num--;
}
void Player::ObtainItem(InventoryItem item, int* num)
{
	bool added = false;
	for (auto& s : itemsort)
	{
		if (s->item == item)
		{
			int spaceleft = pack.sort_max - s->num;
			if (spaceleft > 0)
			{
				if (*num <= spaceleft)
				{
					s->num += *num;
					*num = 0;
					added = true;
				}
				else
				{
					s->num = pack.sort_max;
					*num -= spaceleft;
					added = true;
				}
			}
		}
	}
	if (*num > 0)
	{
		pack.add(item, num);
	}
}

//动画
void Player::RoateBody(float roateY)
{
	GetTransform().SetRotation(0, roateY - XM_PI / 2, 0);
}
void Player::RoateHead(float roateX, float roateY)
{
	float offset = 7;
	GetPartTransform(0).SetRotation(0, 0, 0);
	GetPartTransform(0).SetPosition(0, 0, 0);
	GetPartTransform(0).RotateAround(XMFLOAT3(0, offset, 0),
		XMFLOAT3(0, 0, 1), -roateX);
	GetPartTransform(0).RotateAround(XMFLOAT3(0, offset, 0),
		XMFLOAT3(0, 1, 0), roateY);
}
void Player::WalkAnimation(float handAngle, float legAngle)
{
	//hand
	GetPartTransform(3).SetRotation(0, 0, 0);
	GetPartTransform(3).SetPosition(0, 0, 0);
	GetPartTransform(3).RotateAround(XMFLOAT3(0, 6, 0),
		XMFLOAT3(0, 0, 1), handAngle);

	if (!state.taking_block)
	{
		GetPartTransform(5).SetRotation(0, 0, 0);
		GetPartTransform(5).SetPosition(0, 0, 0);
		GetPartTransform(5).RotateAround(XMFLOAT3(0, 6, 0),
			XMFLOAT3(0, 0, 1), -handAngle);
	}
	else
	{
		GetPartTransform(5).SetRotation(0, 0, 0);
		GetPartTransform(5).SetPosition(0, 0, 0);
		GetPartTransform(5).RotateAround(XMFLOAT3(0, 6, 0),
			XMFLOAT3(0, 0, 1), XM_PI / 6);

		//block
		GetPartTransform(6).SetRotation(0, 0, 0);
		GetPartTransform(6).SetPosition(0, 0, 0);
		GetPartTransform(6).RotateAround(XMFLOAT3(0, 6, 0),
			XMFLOAT3(0, 0, 1), XM_PI / 6);
	}


	//leg
	GetPartTransform(2).SetRotation(0, 0, 0);
	GetPartTransform(2).SetPosition(0, 0, 0);
	GetPartTransform(2).RotateAround(XMFLOAT3(0, 2.5, 0),
		XMFLOAT3(0, 0, 1), -legAngle);

	GetPartTransform(4).SetRotation(0, 0, 0);
	GetPartTransform(4).SetPosition(0, 0, 0);
	GetPartTransform(4).RotateAround(XMFLOAT3(0, 2.5, 0),
		XMFLOAT3(0, 0, 1), legAngle);

	PosOffset.y = -abs(sin(legAngle) * 0.1);
}
void Player::InteractAnimation(float dt)
{
	if (state.interact)
	{
		static float time = 0.5;
		static float omega = XM_2PI / time;
		static float t = 0;
		float angle = sin(omega * t) * XM_PI / 6 + XM_PI / 6;
		t += dt;
		if (t > time)
		{
			state.interact = false;
			t = 0;
		}
		GetPartTransform(5).SetRotation(0, 0, 0);
		GetPartTransform(5).SetPosition(0, 0, 0);
		GetPartTransform(5).RotateAround(XMFLOAT3(0, 6, 0),
			XMFLOAT3(0, 0, 1), angle);

		//block
		GetPartTransform(6).SetRotation(0, 0, 0);
		GetPartTransform(6).SetPosition(0, 0, 0);
		GetPartTransform(6).RotateAround(XMFLOAT3(0, 6, 0),
			XMFLOAT3(0, 0, 1), angle);
	}
}

void Player::WalkFoward()
{
	state.walk_foward = true;
}
void Player::WalkBackward()
{
	state.walk_backward = true;
}
void Player::StrafeLeft()
{
	state.strafe_left = true;
}
void Player::StrafeRight()
{
	state.strafe_right = true;
}
void Player::Interact()
{
	state.interact = true;
}
void Player::UpdateAnimation(float dt)
{

	float body_turn_angle = 0;
	static bool activate_forward = false;
	if (state.walk_foward && state.strafe_left || state.walk_backward && state.strafe_right)
	{
		body_turn_angle = -XM_PI / 4;
	}
	if (state.walk_foward && state.strafe_right || state.walk_backward && state.strafe_left)
	{
		body_turn_angle = XM_PI / 4;
	}
	if (!state.walk_foward && !state.walk_backward && state.strafe_left)
	{
		body_turn_angle = -XM_PI / 2;
		activate_forward = true;
	}
	if (!state.walk_foward && !state.walk_backward && state.strafe_right)
	{
		body_turn_angle = XM_PI / 2;
		activate_forward = true;
	}
	//body turn	
	{
		static float current_angle = 0;
		current_angle += (body_turn_angle - current_angle) * 5 * dt;

		auto roate = GetTransform().GetRotation();
		roate.y += current_angle;
		GetPartTransform(0).RotateAround(XMFLOAT3(0, 7, 0),
			XMFLOAT3(0, 1, 0), -current_angle);
		GetTransform().SetRotation(roate);
		state.strafe_left = false;
		state.strafe_right = false;
	}
	if (state.walk_foward || activate_forward)
	{
		static float time = 1;
		static float omega = XM_2PI / time;
		static float t = 0;
		static bool priod_control = false;
		t += dt;
		if (t > time / 2)
		{
			t = 0;
			state.walk_foward = false;
			activate_forward = false;
			priod_control = !priod_control;
		};
		float angle;
		if (priod_control)
		{
			angle = std::sin((t + 0.5) * omega) * XM_PI / 4;
		}
		else
		{
			angle = std::sin(t * omega) * XM_PI / 4;
		}
		WalkAnimation(angle, angle);
	}
	if (state.walk_backward)
	{
		static float time = 1;
		static float omega = XM_2PI / time;
		static float t = 0;
		static bool priod_control = false;

		t += dt;
		if (t > time / 2)
		{
			t = 0;
			state.walk_backward = false;
			priod_control = !priod_control;
		};
		float angle;
		if (priod_control)
		{
			angle = std::sin((t + 0.5) * omega) * XM_PI / 4;
		}
		else
		{
			angle = std::sin(t * omega) * XM_PI / 4;
		}
		WalkAnimation(angle, angle);
	}

	InteractAnimation(dt);

	UpdateHandingCube();
}


void Player::UpdateHandingCube()
{
	static BlockType taking = BlockType::Null;
	BlockType block = GetBlockType();
	if (taking != block)
	{
		if (block != BlockType::Null)
		{
			if (BlockRender::GetInstance())
				m_Model.modelParts[6].texDiffuse = BlockRender::GetInstance()->GetTexture(block);
			taking = block;
			m_Model.modelParts[6].visible = true;
			state.taking_block = true;

			//初始化动画
			GetPartTransform(5).SetRotation(0, 0, 0);
			GetPartTransform(5).SetPosition(0, 0, 0);
			GetPartTransform(5).RotateAround(XMFLOAT3(0, 6, 0),
				XMFLOAT3(0, 0, 1), XM_PI / 6);

			//block
			GetPartTransform(6).SetRotation(0, 0, 0);
			GetPartTransform(6).SetPosition(0, 0, 0);
			GetPartTransform(6).RotateAround(XMFLOAT3(0, 6, 0),
				XMFLOAT3(0, 0, 1), XM_PI / 6);
		}
		else
		{
			GetPartTransform(5).SetRotation(0, 0, 0);
			GetPartTransform(5).SetPosition(0, 0, 0);
			GetPartTransform(6).SetRotation(0, 0, 0);
			GetPartTransform(6).SetPosition(0, 0, 0);
			taking = block;
			m_Model.modelParts[6].visible = false;
			state.taking_block = false;
		}
	}
}

void Player::FirstPersonModel()
{
	state.thrid_person = false;
	for (auto& part : m_Model.modelParts)
	{
		part.visible = false;
	}
	m_Model.modelParts[5].visible = true;
	if (state.taking_block)m_Model.modelParts[6].visible = true;
}
void Player::ThirdPersonModel()
{
	state.thrid_person = true;
	for (auto& part : m_Model.modelParts)
	{
		part.visible = true;
	}
	if (!state.taking_block)m_Model.modelParts[6].visible = false;
}

//重生
void Player::Death()
{
	pack.clear();
	HitPoint = MaxHitPoint;
	shouldRemove = false;
	GetPosition() = BirthPlace;
}

//扫描周围生物
void Player::ScanCreature()
{
	for (auto& phy_obj : Objects)
	{
		auto target_ptr = dynamic_cast<Animal*>(phy_obj.second);
		if (target_ptr)
		{
			if (target_ptr->InRange(CreaturePos))
			{

			}
		}
	}
}