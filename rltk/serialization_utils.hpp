#pragma once

#include <fstream>
#include <string>
#include <vector>
#include "color_t.hpp"

namespace rltk {

template<class T>
inline void serialize(std::ostream &lbfile, const T &target) {
	lbfile.write(reinterpret_cast<const char *>(&target), sizeof(target));
}
template<>
inline void serialize(std::ostream &lbfile, const std::string &target) {
	std::size_t size = target.size();
	serialize<std::size_t>(lbfile, size);
	for (std::size_t i = 0; i < size; ++i)
	{
		serialize<char>(lbfile, target[i]);
	}
}
template<>
inline void serialize(std::ostream &lbfile, const rltk::color_t &col) {
	serialize<uint8_t>(lbfile, col.r);
	serialize<uint8_t>(lbfile, col.g);
	serialize<uint8_t>(lbfile, col.b);
}

template<class T>
inline void serialize(std::ostream &lbfile, const std::vector<T> &vec) {
	std::size_t sz = vec.size();
	serialize(lbfile, sz);
	for (std::size_t i=0; i<sz; ++i) {
		serialize(lbfile, vec[i]);
	}
}
template<>
inline void serialize(std::ostream &lbfile, const std::vector<bool> &vec) {
	std::size_t sz = vec.size();
	serialize(lbfile, sz);
	for (std::size_t i=0; i<sz; ++i) {
		bool b = vec[i];
		serialize(lbfile, b);
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
	std::size_t size = 0;
	deserialize<std::size_t>(lbfile, size);
	for (std::size_t i = 0; i < size; ++i)
	{
		char c;
		deserialize<char>(lbfile, c);
		result += c;
	}
	target = result;
}
template<>
inline void deserialize(std::istream &lbfile, rltk::color_t &target) {
	deserialize(lbfile, target.r);
	deserialize(lbfile, target.g);
	deserialize(lbfile, target.b);
}
template<class T>
inline void deserialize(std::istream &lbfile, std::vector<T> &vec) {
	std::size_t size;
	deserialize(lbfile, size);
	vec.resize(size);
	for (std::size_t i=0; i<size; ++i) {
		T tmp;
		deserialize(lbfile, tmp);
		vec[i] = tmp;
	}
}

}