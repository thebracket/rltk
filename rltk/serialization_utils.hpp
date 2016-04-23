#pragma once

#include <fstream>
#include <string>

namespace rltk {

template<class T>
inline void serialize(std::ostream &lbfile, const T &target) {
	lbfile.write(reinterpret_cast<const char *>(&target), sizeof(target));
}
template<>
inline void serialize(std::ostream &lbfile, const std::string &target) {
	unsigned int size = target.size();
	serialize<std::size_t>(lbfile, size);
	for (unsigned int i = 0; i < size; ++i)
	{
		serialize<char>(lbfile, target[i]);
	}
}

template<typename T>
inline void deserialize(std::istream &lbfile, T &target)
{
	lbfile.read(reinterpret_cast<char *>(&target), sizeof(target));
}
template<>
inline void deserialize(std::istream &lbfile, std::string &target)
{
	std::string result;
	unsigned int size = 0;
	deserialize<unsigned int>(lbfile, size);
	for (std::size_t i = 0; i < size; ++i)
	{
		char c;
		deserialize<char>(lbfile, c);
		result += c;
	}
	target = result;
}

}