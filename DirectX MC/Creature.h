#pragma once
#include"GameObject.h"
#include"Physic.h"
class Creature :public GameObject, public Physic
{
	float max_speed = 3;

	void movingAnimation();

	bool jumped = false;

	
public:
	XMFLOAT3 CreaturePos;
	XMFLOAT3 PosOffset{};
	XMFLOAT3 FaceDirection;

	int MaxHitPoint = 100;
	int HitPoint = MaxHitPoint;
	bool shouldRemove = false;
	bool shouldRender = false;
	//伤害
	int basic_damage = 10;
	int damage = basic_damage;


	Creature() :Physic(&CreaturePos) {};

	XMFLOAT3& GetPosition();
	XMFLOAT3 GetPositionWithOffset();

	void Update(float dt);

	void Move(const XMFLOAT3& dir, float dt);
	void Jump(float dt);
	void SetMoveSpeed(float newSpeed);

	void Hit(int damge);
	void Attack(Creature* target, float dt);
	void EnhanceDamage(float percentage);
	void ResetDamage();
	void AddDamage(int add);

};

