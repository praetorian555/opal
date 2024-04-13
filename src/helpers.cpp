#include "helpers.h"

int32_t GetRandomInt(int32_t lo, int32_t hi)
{
	static std::mt19937_64 gen;
	std::uniform_int_distribution<int32_t> dist(lo, hi);
	return dist(gen);
}

UniformRandomIntGenerator::UniformRandomIntGenerator(int32_t seed) : m_generator(seed)
{
}

int32_t UniformRandomIntGenerator::GetRandomInt(int32_t lo, int32_t hi)
{
	std::uniform_int_distribution<int32_t> dist(lo, hi);
	return dist(m_generator);
}

std::vector<int32_t> UniformRandomIntGenerator::GetRandomIntArray(int32_t size, int32_t lo, int32_t hi)
{
	std::vector<int32_t> vec;
	for (int i = 0; i < size; i++)
	{
		vec.push_back(GetRandomInt(lo, hi));
	}
	return vec;
}
