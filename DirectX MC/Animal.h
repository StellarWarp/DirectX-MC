#pragma once
#include"Creature.h"
class Animal :public Creature
{
	bool is_walk;
	XMFLOAT3 MoveDirection;
	XMFLOAT3* PlayerPos = nullptr;
	static Model* Animal_Model;
	static int num;

	//动画参数
	struct animation_value
	{
		float current_angle = 0;
		float t1 =0;
		bool priod_control = false;
		float ph_show_percentage = 0;
		float hp_value = 1;

		float wander_T = 3;
		float wander_t = 0;
		float wander_dir1 = 1;
		float wander_dir2 = 1;
	}animval;


public:

	void init_Animal(ID3D11Device* device, World* world);

	void init_Animal(World* world);

	void Load(WorldDesc::CreatureDesc desc, World* world);

	void Save(WorldDesc::CreatureDesc* desc);


	void Wander(float dt);
	void Process(float dt);

	void WalkAnimation(float legAngle);
	void UpdateAnimation(float dt);

	void UpdateHp(float bodyRoateAngle);
	bool InRange(XMFLOAT3& PlayerPos);

	void HpModel(bool on);

	int GetNumber();

	Animal()
	{
		if (Animal_Model)
		{
			m_Model = *Animal_Model;
		}
	}
	~Animal()
	{

	}

};

