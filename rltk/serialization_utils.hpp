#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <zlib.h>
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

struct gzip_file {
	gzip_file(const std::string fn, const char* perms) : filename(fn), permissions(perms) {
		file = gzopen(filename.c_str(), permissions);

		if (file == Z_NULL) {
			int err = 0;
			const char* errstr = gzerror(file, &err);
			if (err == 0) {
				/*Assume the file simply didn't exist.*/
				std::string s("File " + filename + " does not exist.");
				throw std::runtime_error(s);
			}
			throw std::runtime_error(std::string(errstr));
		}
	}

	~gzip_file() {
		gzclose(file);
	}

	inline void throw_gzip_exception(gzFile &gzfile) {
		int errnum = 0;
		const char* errstr = gzerror(gzfile, &errnum);
		throw std::runtime_error(std::to_string(errnum) + std::string(" ") + std::string(errstr));
	}

	template<class T>
	inline void serialize(const T &target) {
		if (gzwrite(file, reinterpret_cast<const char *>(&target), sizeof(target))) return;
		throw_gzip_exception(file);
	}
	template<class T>
	inline void serialize(const std::string &target) {
		std::size_t size = target.size();
		serialize<std::size_t>(size);
		for (std::size_t i = 0; i < size; ++i)
		{
			serialize<char>(target[i]);
		}
	}
	template<class T>
	inline void serialize(const rltk::color_t &col) {
		serialize<uint8_t>(col.r);
		serialize<uint8_t>(col.g);
		serialize<uint8_t>(col.b);
	}
	inline void serialize_vector_bool(const std::vector<bool> &vec) {
		std::size_t sz = vec.size();
		serialize(sz);
		for (std::size_t i=0; i<sz; ++i) {
			bool b = vec[i];
			serialize<bool>(b);
		}
	}
	template<class T>
	inline void serialize(const std::vector<T> &vec) {
		std::size_t sz = vec.size();
		serialize(sz);
		for (std::size_t i=0; i<sz; ++i) {
			serialize(vec[i]);
		}
	}

	template<typename T>
	inline void deserialize(T &target)
	{
		if (gzread(file, reinterpret_cast<char *>(&target), sizeof(target))) return;
		throw_gzip_exception(file);
	}
	template<typename T>
	inline void deserialize(std::string &target)
	{
		std::string result;
		std::size_t size = 0;
		deserialize<std::size_t>(size);
		for (std::size_t i = 0; i < size; ++i)
		{
			char c;
			deserialize<char>(c);
			result += c;
		}
		target = result;
	}
	template<typename T>
	inline void deserialize(rltk::color_t &target) {
		deserialize(target.r);
		deserialize(target.g);
		deserialize(target.b);
	}
	template<class T>
	inline void deserialize(std::vector<T> &vec) {
		std::size_t size;
		deserialize(size);
		vec.resize(size);
		for (std::size_t i=0; i<size; ++i) {
			T tmp;
			deserialize(tmp);
			vec[i] = tmp;
		}
	}

private:
	const std::string filename;
	const char* permissions;
	gzFile file;
};

}