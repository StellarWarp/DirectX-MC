#pragma once
#include<array>
#include <algorithm>

template<class T,int WIDTH,int HIGHT>
class Array3D
{
	using Array = std::array<T, WIDTH* WIDTH* HIGHT>;
	int width_2 = WIDTH * WIDTH;

	Array m_array;
public:
	Array& GetArray()
	{
		return m_array;
	}
	Array3D(T fillValue)
	{
		m_array.fill(fillValue);
	}
	T& get(int x, int y, int z)
	{
		return m_array[y * width_2 + x * WIDTH + z];
	}

	const T& get(int x, int y, int z) const
	{
		return m_array[y * width_2 + x * WIDTH + z];
	}

	T& maxValue()
	{
		return *std::max_element(m_array.begin(), m_array.end());
	}
};

