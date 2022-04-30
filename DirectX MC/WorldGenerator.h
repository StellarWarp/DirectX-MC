#pragma once
#include <vector>
#include <iostream>
#include "Block.h"
#include "World.h"

// 地图长宽高 640 256 256
constexpr auto WORLD_WIDTH = 100;
constexpr auto WORLD_HEIGHT = 100;
// 雪地面积
constexpr auto SNOW_RADIUS = 25;

using namespace std;

using vec3 = vector<vector<vector<BlockType>>>;

// 随机创建地图类
class WorldGenerator
{
public:
	static WorldGenerator* GetInstance();
	// 初始化地图容器
	void initMapVec();
	// 创建方块
	void CreateCubeMap();
	// 创建植物
	void CreatePlantMap();
	// 创建树
	void CreateTreeMap();
	// 所在地图位置的海拔
	int getAltitude(int x, int z, int y);
public:
	// 方块位置数据
	vec3 mapData;
	// 植物位置数据
	vec3 plantData;
	// 树叶位置数据
	vec3 leafData;
private:
	WorldGenerator();
	WorldGenerator(const WorldGenerator&) = delete;
	WorldGenerator& operator = (const WorldGenerator&) = delete;

	// 前海拔 (创建随机地图时使用)
	vector<int> beforeAltitudeVec;
	// 地图海拔 (创建随机地图时使用)
	vector<vector<int>> altitudeMapVec;

	// 地图第一行的海拔随机初始化
	void randInitAltitude();
	// 地图中一行海拔数据的初始化
	void randLineAltitude();
	// 随机初始化地图的所有海拔
	void randAllAltitude();
	// 地图海拔平均化
	void avgAltitude();
	// 填充草地
	void fillingGrasslandCube();
	// 随机创建雪地
	void RandSnowCubeMap();
	// 周围是否存在树
	bool isAroundTree(int x, int z);
	// 填充树叶
	void fillingLeave(int x, int z, int height, BlockType ct);
};