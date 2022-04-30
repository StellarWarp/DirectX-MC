#pragma once
#include"World.h"
#include"GameObject.h"
#include <d3dApp.h>

class Physic
{
	DirectX::XMFLOAT3* pos;
	XMFLOAT3 pos_offset;
	XMINT3 world_pos;

	//shape desc
	float hight = 1.8;
	float radius = 0.2;
	float s = 0.5f - radius;

	std::array<XMFLOAT3, 4> edge_pos;

	float hitforce = 10000;
public:
	static UINT objects_count;
	static std::unordered_map<UINT, Physic*> Objects;
	UINT ID;
protected:
	World* m_pWorld;

	//physic
	float m_mass = 2;
	XMFLOAT3 m_speed{};
	XMFLOAT3 m_force{};

	bool onGound = false;
	bool tryJump = false;

	Physic(XMFLOAT3* pos);
	~Physic()
	{
		Objects.erase(ID);

		auto impl = m_pWorld->GetChunk();
		auto& block1 = impl->GetBlock(world_pos);
		world_pos.y += 1;
		auto& block2 = impl->GetBlock(world_pos);
		block1.RemoveObj(ID);
		block2.RemoveObj(ID);
	}
	void init_Physic(World* world);

	bool HasGound();
	bool HasCell();

	void NormalProcess(float dt);

	void Collision(float dt);

	//无用
	void virtual unuse();

public:

	void ApplyForce(const XMFLOAT3& _force);
	void UpdatePhysic(float dt);

	World* GetWorld()
	{
		return m_pWorld;
	}

};

class PhysicEngine
{
	std::unordered_map <UINT64, Physic> Objects;

};

