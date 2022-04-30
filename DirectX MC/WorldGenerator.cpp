#include "WorldGenerator.h"
#include <ctime>
#include <random>


WorldGenerator* WorldGenerator::GetInstance()
{
	static WorldGenerator createWorld;
	return &createWorld;
}

WorldGenerator::WorldGenerator()
{
}

void WorldGenerator::CreateCubeMap()
{
	randInitAltitude();
	randAllAltitude();
	avgAltitude();

	fillingGrasslandCube();

}

static std::random_device r;
static std::default_random_engine e1(r());
static std::uniform_int_distribution<int> uniform_altitude(-1, 1);
static std::uniform_int_distribution<int> uniform_snow(-5, 3);
static std::uniform_int_distribution<int> uniform_snow_region(-SNOW_RADIUS, SNOW_RADIUS);
static std::uniform_int_distribution<int> uniform_value(0, WORLD_WIDTH - 1);


void WorldGenerator::randAllAltitude()
{
	for (int x = 0; x < WORLD_WIDTH; x++)
	{
		altitudeMapVec.push_back(beforeAltitudeVec);
		randLineAltitude();
	}
}

void WorldGenerator::randLineAltitude()
{
	int rand_num = 0;
	int altitude = 0;
	double avg1 = 0;
	double avg2 = 0;
	for (int i = 1; i < WORLD_WIDTH - 1; i++)
	{
		rand_num = uniform_altitude(e1);
		avg1 = double(beforeAltitudeVec[i] + beforeAltitudeVec[i - 1] + beforeAltitudeVec[i + 1]) / 3;
		avg2 = double(avg1 + rand_num + beforeAltitudeVec[i - 1] + beforeAltitudeVec[i + 1]) / 3;

		if (avg1 == avg2)
		{
			altitude = avg2;
		}
		else if (avg1 > avg2)
		{
			altitude = avg2;
		}
		else if (avg1 < avg2)
		{
			altitude = avg2 + 1;
		}

		if (altitude <= 0 || altitude >= 200)
			altitude = 100;

		beforeAltitudeVec[i] = altitude;
	}
}

void WorldGenerator::avgAltitude()
{
	int avg = 0;
	for (int x = 0; x < WORLD_WIDTH; x++)
	{
		for (int z = WORLD_WIDTH; z > 0; z--)
		{
			avg = 0;
			int count = 5;

			if (x + 2 < WORLD_WIDTH && z - 2 > 0 && z + 2 < WORLD_WIDTH)
			{
				avg += altitudeMapVec[x + 2][z - 2];	avg += altitudeMapVec[x + 2][z - 1];	avg += altitudeMapVec[x + 2][z + 0];	avg += altitudeMapVec[x + 2][z + 1];	avg += altitudeMapVec[x + 2][z + 2];
			}
			else
			{
				count -= 1;
			}

			if (x + 1 < WORLD_WIDTH && z - 2 > 0 && z + 2 < WORLD_WIDTH)
			{
				avg += altitudeMapVec[x + 1][z - 2];	avg += altitudeMapVec[x + 1][z - 1];	avg += altitudeMapVec[x + 1][z + 0];	avg += altitudeMapVec[x + 1][z + 1];	avg += altitudeMapVec[x + 1][z + 2];
			}
			else
			{
				count -= 1;
			}

			if (z - 2 > 0 && z + 2 < WORLD_WIDTH)
			{
				avg += altitudeMapVec[x + 0][z - 2];	avg += altitudeMapVec[x + 0][z - 1];	avg += altitudeMapVec[x + 0][z + 0];	avg += altitudeMapVec[x + 0][z + 1];	avg += altitudeMapVec[x + 0][z + 2];
			}
			else
			{
				count -= 1;
			}

			if (x - 1 > 0 && z - 2 > 0 && z + 2 < WORLD_WIDTH)
			{
				avg += altitudeMapVec[x - 1][z - 2];	avg += altitudeMapVec[x - 1][z - 1];	avg += altitudeMapVec[x - 1][z + 0];	avg += altitudeMapVec[x - 1][z + 1];	avg += altitudeMapVec[x - 1][z + 2];
			}
			else
			{
				count -= 1;
			}

			if (x - 2 > 0 && z - 2 > 0 && z + 2 < WORLD_WIDTH)
			{
				avg += altitudeMapVec[x - 2][z - 2];	avg += altitudeMapVec[x - 2][z - 1];	avg += altitudeMapVec[x - 2][z + 0];	avg += altitudeMapVec[x - 2][z + 1];	avg += altitudeMapVec[x - 2][z + 2];
			}
			else
			{
				count -= 1;
			}

			if (count > 0)
			{
				avg = avg / (count * 5);
				altitudeMapVec[x][z] = avg;
			}
		}
	}
}

void WorldGenerator::fillingGrasslandCube()
{
	for (int x = 0; x < WORLD_WIDTH; x++)
	{
		for (int z = 0; z < WORLD_WIDTH; z++)
		{
			mapData[x][altitudeMapVec[x][z]][z] = BlockType::MapGrassland;
			for (int y = 0; y < altitudeMapVec[x][z]; y++)
				mapData[x][y][z] = BlockType::MapLand;
		}
	}
}

void WorldGenerator::RandSnowCubeMap()
{
	int centerX = uniform_value(e1);
	int centerZ = uniform_value(e1);

	for (int z = centerZ - SNOW_RADIUS; z < centerZ + SNOW_RADIUS; z++)
	{
		for (int x = centerX - SNOW_RADIUS; x < centerX + SNOW_RADIUS; x++)
		{
			if (x > 1 && z > 1 && x < WORLD_WIDTH - 1 && z < WORLD_WIDTH - 1)
			{
				int xx = x - (centerX - SNOW_RADIUS);
				int zz = z - (centerZ - SNOW_RADIUS);

				if (xx / SNOW_RADIUS == 0 && zz / SNOW_RADIUS == 0)
				{
					if (xx % SNOW_RADIUS > (SNOW_RADIUS - SNOW_RADIUS / 4 - zz % SNOW_RADIUS))
					{
						int rand = uniform_snow(e1);
						if (rand == -5)
							mapData[x - 1][altitudeMapVec[x - 1][z]][z] = BlockType::MapSnow;
						else if (rand == 3)
							mapData[x][altitudeMapVec[x][z - 1]][z - 1] = BlockType::MapSnow;

						mapData[x][altitudeMapVec[x][z]][z] = BlockType::MapSnow;
					}
				}
				else if (xx / SNOW_RADIUS == 1 && zz / SNOW_RADIUS == 0)
				{
					if (xx % SNOW_RADIUS < SNOW_RADIUS / 4 + zz % SNOW_RADIUS)
					{
						int rand = uniform_snow(e1);
						if (rand == -5)
							mapData[x + 1][altitudeMapVec[x + 1][z]][z] = BlockType::MapSnow;
						else if (rand == 3)
							mapData[x][altitudeMapVec[x][z - 1]][z - 1] = BlockType::MapSnow;

						mapData[x][altitudeMapVec[x][z]][z] = BlockType::MapSnow;
					}
				}
				else if (xx / SNOW_RADIUS == 0 && zz / SNOW_RADIUS == 1)
				{
					if (xx % SNOW_RADIUS > zz % SNOW_RADIUS - SNOW_RADIUS / 4)
					{
						int rand = uniform_snow(e1);
						if (rand == -5)
							mapData[x - 1][altitudeMapVec[x - 1][z]][z] = BlockType::MapSnow;
						else if (rand == 3)
							mapData[x][altitudeMapVec[x][z + 1]][z + 1] = BlockType::MapSnow;

						mapData[x][altitudeMapVec[x][z]][z] = BlockType::MapSnow;
					}
				}
				else if (xx / SNOW_RADIUS == 1 && zz / SNOW_RADIUS == 1)
				{
					if (xx % SNOW_RADIUS < (SNOW_RADIUS + SNOW_RADIUS / 4 - zz % SNOW_RADIUS))
					{
						int rand = uniform_snow(e1);
						if (rand == -5)
							mapData[x + 1][altitudeMapVec[x + 1][z]][z] = BlockType::MapSnow;
						else if (rand == 3)
							mapData[x][altitudeMapVec[x][z - 1]][z + 1] = BlockType::MapSnow;

						mapData[x][altitudeMapVec[x][z]][z] = BlockType::MapSnow;
					}
				}

			}
		}
	}

}

void WorldGenerator::initMapVec()
{
	mapData.resize(WORLD_WIDTH, vector<vector<BlockType>>(WORLD_HEIGHT, vector<BlockType>(WORLD_WIDTH, BlockType::Null)));
	plantData.resize(WORLD_WIDTH, vector<vector<BlockType>>(WORLD_HEIGHT, vector<BlockType>(WORLD_WIDTH, BlockType::Null)));
	leafData.resize(WORLD_WIDTH, vector<vector<BlockType>>(WORLD_HEIGHT, vector<BlockType>(WORLD_WIDTH, BlockType::Null)));
}

void WorldGenerator::randInitAltitude()
{
	beforeAltitudeVec.reserve(WORLD_WIDTH);
	int rand_num = 0;
	int num_old = 0;
	int altitude = WORLD_HEIGHT / 2;
	for (int i = 0; i < WORLD_WIDTH; i++)
	{
		num_old = rand_num;
		rand_num = uniform_altitude(e1);

		if (rand_num == -1)
		{
			if (num_old == 1)
				rand_num = 0;
		}
		else if (rand_num == 1)
		{
			if (num_old == -1)
				rand_num = 0;
		}

		altitude += rand_num;

		if (altitude <= 0 || altitude >= 200)
			altitude = 100;

		beforeAltitudeVec.push_back(WORLD_HEIGHT / 2);
	}
}

void WorldGenerator::CreateTreeMap()
{
	int x = 0;
	int z = 0;
	for (int region = 0; region < (WORLD_WIDTH / 25) * 2; region++)
	{
		int xx = uniform_value(e1) % (WORLD_WIDTH - WORLD_WIDTH / 4);
		int zz = uniform_value(e1) % (WORLD_WIDTH - WORLD_WIDTH / 4);
		for (int count = 0; count < WORLD_WIDTH / 8; count++)
		{
			x = uniform_value(e1) % (WORLD_WIDTH / 4) + xx;
			z = uniform_value(e1) % (WORLD_WIDTH / 4) + zz;

			if (x >= WORLD_WIDTH || z >= WORLD_WIDTH)
				continue;

			if (plantData[x][altitudeMapVec[x][z] + 1][z] == BlockType::Null)
			{
				if (!isAroundTree(x, z))
				{
					int height = uniform_value(e1) % 3 + 5 + altitudeMapVec[x][z];

					fillingLeave(x, z, height, BlockType::MapLeaveGreen);

					for (int y = altitudeMapVec[x][z]; y < height; y++)
					{
						altitudeMapVec[x][z] += 1;
						mapData[x][altitudeMapVec[x][z]][z] = BlockType::MapTree;
					}
				}
			}
		}
	}


	int count = 0;
	do
	{
		int x = uniform_value(e1);
		int z = uniform_value(e1);

		if (plantData[x][altitudeMapVec[x][z] + 1][z] == BlockType::Null)
		{
			if (mapData[x][altitudeMapVec[x][z]][z] != BlockType::MapSnow)
			{
				if (!isAroundTree(x, z))
				{
					int height = uniform_value(e1) % 3 + 5 + altitudeMapVec[x][z];

					if (count % 2 == 0)
						fillingLeave(x, z, height, BlockType::MapLeaveRed);
					else
						fillingLeave(x, z, height, BlockType::MapLeaveYellow);

					for (int y = altitudeMapVec[x][z]; y < height; y++)
					{
						altitudeMapVec[x][z] += 1;
						mapData[x][altitudeMapVec[x][z]][z] = BlockType::MapTree;
					}

					count++;
				}
			}
		}
	} while (count < WORLD_WIDTH/2);
}

int WorldGenerator::getAltitude(int x, int z, int y)
{
	for (int i = y; i > 0; i--)
	{
		if (mapData[x][i][z] != BlockType::Null)
			return i + 1;
	}

	return 0;
}

bool WorldGenerator::isAroundTree(int x, int z)
{
	for (int xx = x - 4; xx <= x + 4; xx++)
	{
		for (int zz = z - 4; zz <= z + 4; zz++)
		{
			if (xx >= 0 && xx < WORLD_WIDTH && zz >= 0 && zz < WORLD_WIDTH)
			{
				if (mapData[xx][altitudeMapVec[xx][zz]][zz] == BlockType::MapTree)
					return true;
			}
		}
	}
	return false;
}

void WorldGenerator::fillingLeave(int x, int z, int height, BlockType ct)
{
	if (x > 1 && x < WORLD_WIDTH - 1 && z >1 && z < WORLD_WIDTH)
	{
		if (mapData[x - 1][altitudeMapVec[x - 1][z + 0]][z + 0] == BlockType::MapSnow &&
			mapData[x + 1][altitudeMapVec[x + 1][z + 0]][z + 0] == BlockType::MapSnow &&
			mapData[x + 0][altitudeMapVec[x + 0][z - 1]][z - 1] == BlockType::MapSnow &&
			mapData[x + 0][altitudeMapVec[x + 0][z + 1]][z + 1] == BlockType::MapSnow)
		{
			ct = BlockType::MapLeaveWhite;
		}
	}

	for (int xx = x - 1; xx <= x + 1; xx++)
	{
		for (int zz = z - 1; zz <= z + 1; zz++)
		{
			if (xx >= 0 && xx < WORLD_WIDTH && zz >= 0 && zz < WORLD_WIDTH)
			{
				leafData[xx][height + 0][zz] = ct;
				if (xx == x || zz == z)
					leafData[xx][height + 1][zz] = ct;
			}
		}
	}

	for (int xx = x - 2; xx <= x + 2; xx++)
	{
		for (int zz = z - 2; zz <= z + 2; zz++)
		{
			if (xx >= 0 && xx < WORLD_WIDTH && zz >= 0 && zz < WORLD_WIDTH)
			{
				leafData[xx][height - 1][zz] = ct;
				leafData[xx][height - 2][zz] = ct;
			}
		}
	}

	if (height - altitudeMapVec[x][z] >= 7)
	{
		for (int xx = x - 1; xx <= x + 1; xx++)
		{
			for (int zz = z - 1; zz <= z + 1; zz++)
			{
				if (xx >= 0 && xx < WORLD_WIDTH && zz >= 0 && zz < WORLD_WIDTH)
				{
					leafData[xx][height - 3][zz] = ct;
				}
			}
		}
	}
}

