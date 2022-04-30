#pragma once
#include"Creature.h"
#include"Block.h"
#include<array>
#include<algorithm>
#include <Animal.h>
#include"InventoryItem.h"
class Player :public Creature
{
	struct sort
	{
		InventoryItem item = InventoryItem::Null;
		UINT16 num = 0;
	};
	struct Inventory
	{
		const UINT sort_max = 64;
		const UINT Inventory_max = 20;
		std::array<sort, 10>data;
		void add(InventoryItem item, int* num)
		{
			bool added = false;
			for (auto& s : data)
			{
				if (s.item == item)
				{
					int spaceleft = sort_max - s.num;
					if (spaceleft > 0)
					{
						if (*num <= spaceleft)
						{
							s.num += *num;
							*num = 0;

							added = true;
							return;
						}
						else
						{
							s.num = sort_max;
							*num -= spaceleft;
							added = true;
						}
					}
				}
			}

			while (*num > 0)
			{
				for (auto& s : data)
				{
					if (s.item == InventoryItem::Null)
					{
						s.item = item;
						if (*num > sort_max)
						{
							s.num = sort_max;
							*num -= sort_max;
							continue;
						}
						else
						{
							s.num = *num;
							*num = 0;
							return;
						}

					}
				}
				//full
				break;
			}

		}
		void put(InventoryItem item, int* num);
		void clear()
		{

		}
	};
	struct AnimationState
	{
		bool walk_foward = false;
		bool walk_backward = false;
		bool run = false;
		bool strafe_left = false;
		bool strafe_right = false;
		bool taking_block = false;
		bool thrid_person = false;
		bool interact = false;
	};
	std::vector<InventoryItem>InventoryItem_iter;
public:
	//储物系统
	Inventory pack;
	std::array<sort*, 10> itemsort;
	sort* currentsort;
	//动画
	AnimationState state;
	//重生
	XMFLOAT3 BirthPlace;



	Player()
	{
		itemsort.fill(nullptr);
		for (UINT i = 0; i < 10; i++)
		{
			itemsort[i] = &pack.data[i];
		}
		currentsort = itemsort[0];
	}
	void init_Player(ID3D11Device* device, World* world);

	void Load(WorldDesc::PlayerDesc desc);

	void Save(WorldDesc::PlayerDesc* desc);



	//储物
	void SelectSort(int i);
	InventoryItem GetItem();
	BlockType GetBlockType();
	void UseItem();
	void ObtainItem(InventoryItem item, int* num);

	//动画
	void RoateBody(float roateY);
	void RoateHead(float roateX, float roateY);
	void WalkAnimation(float handAngle, float legAngle);
	void InteractAnimation(float dt);
	void WalkFoward();
	void WalkBackward();
	void StrafeLeft();
	void StrafeRight();
	void Interact();
	void UpdateAnimation(float dt);

	void UpdateHandingCube();

	void FirstPersonModel();
	void ThirdPersonModel();

	//重生
	void Death();

	//扫描周围生物
	void ScanCreature();
};

