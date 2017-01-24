#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <zlib.h>
#include <utility>
#include <sstream>
#include "color_t.hpp"
#include "xml.hpp"
#include "vchar.hpp"

namespace rltk {

template<typename... T>
inline void component_to_xml(xml_node * c, const T... args);

namespace serial {

// Forward declarations
template<typename... T>
inline void component_to_xml(xml_node * c, const T... args);

template <typename First, typename... Rest>
void _component_to_xml(xml_node * c, First arg, Rest... args);

// Final render to string
template <class T>
inline std::string to_string(T val) {
	std::stringstream ss;
	ss << val;
	return ss.str();
}
template <>
inline std::string to_string(uint8_t val) {
	std::stringstream ss;
	ss << static_cast<int>(val);
	return ss.str();
}

template <class T>
void __component_to_xml(xml_node *c, T arg) {
	c->add_value(arg.first, rltk::serial::to_string(arg.second));
}

template< class T >
struct has_to_xml_method
{
    typedef char(&YesType)[1];
    typedef char(&NoType)[2];
    template< class, class > struct Sfinae;

    template< class T2 > static YesType Test( Sfinae<T2, decltype(std::declval<T2>().to_xml( std::declval<xml_node *>() ))> * );
    template< class T2 > static NoType  Test( ... );
    static const bool value = sizeof(Test<T>(0))==sizeof(YesType);
};

/* Uses enable_if to determine if it should call a struct's load method, or just pass through to
 * a more specific Deserialize */
template<class T>
struct _component_to_xml_check_for_to_xml
{
    template<class Q = T>
	typename std::enable_if< has_to_xml_method<Q>::value, void >::type
    test(xml_node * c, std::pair<const char *,Q> &arg)
    {
		arg.second.to_xml(c);
    }

    template<class Q = T>
	typename std::enable_if< !has_to_xml_method<Q>::value, void >::type
    test(xml_node * c, std::pair<const char *,Q> &arg)
    {
        __component_to_xml(c, arg);
    }
};

template <typename T>
inline void _component_to_xml(xml_node * c, std::pair<const char *,T> arg) {
	_component_to_xml_check_for_to_xml<T> temp;
	temp.test(c, arg);
}
template<>
inline void _component_to_xml(xml_node * c, std::pair<const char *,rltk::color_t> arg) {
	// Serialize r/g/b components
	xml_node * ch = c->add_node(arg.first);
	ch->add_value("r", std::to_string(arg.second.r));
	ch->add_value("g", std::to_string(arg.second.g));
	ch->add_value("b", std::to_string(arg.second.b));
}
template<>
inline void _component_to_xml(xml_node * c, std::pair<const char *,rltk::vchar> arg) {
	// Serialize r/g/b components
	xml_node * ch = c->add_node(arg.first);
	ch->add_value("glyph", std::to_string(arg.second.glyph));
	_component_to_xml(ch, std::make_pair("foreground", arg.second.foreground));
	_component_to_xml(ch, std::make_pair("background", arg.second.background));
}

template<typename T>
inline void _component_to_xml(xml_node *c, std::pair<const char *, std::vector<T>> arg) {
	xml_node * vec = c->add_node(arg.first);
	for (T item : arg.second) {
		rltk::component_to_xml(vec, std::make_pair(arg.first, item));
	}
}

template<typename T, typename S>
inline void _component_to_xml(xml_node *c, std::pair<const char *, std::vector<std::pair<T,S>>> arg) {
	xml_node * vec = c->add_node(arg.first);
	for (auto item : arg.second) {
		xml_node * e = vec->add_node(arg.first);
		xml_node * i = e->add_node(std::string(arg.first) + std::string("_first"));
		rltk::component_to_xml(i, std::make_pair(arg.first, item.first));
		xml_node * i2 = e->add_node(std::string(arg.first) + std::string("_second"));
		rltk::component_to_xml(i2, std::make_pair(arg.first, item.second));
	}
}

template<typename T, typename S>
inline void _component_to_xml(xml_node *c, std::pair<const char *, std::unordered_set<T,S>> arg) {
	xml_node * set = c->add_node(arg.first);
	for (auto it = arg.second.begin(); it != arg.second.end(); ++it) {
        xml_node * key = set->add_node("key");
        rltk::component_to_xml(key, std::make_pair("k", *it));
    }
}

template<typename T, typename S>
inline void _component_to_xml(xml_node *c, std::pair<const char *, std::unordered_map<T,S>> arg) {
    xml_node * map = c->add_node(arg.first);
    for (auto it = arg.second.begin(); it != arg.second.end(); ++it) {
        xml_node * entry = map->add_node(arg.first);
        entry->add_value("key", to_string(it->first));
        rltk::component_to_xml(entry, std::make_pair("v", it->second));
    }
}

template<typename T>
inline void _component_to_xml(xml_node * c, std::pair<const char *, std::unique_ptr<T>> arg) {
    xml_node * optional = c->add_node(arg.first);
    if (!arg.second) {
        optional->add_value("initialized", "no");
    } else {
        optional->add_value("initialized", "yes");
        rltk::component_to_xml(optional, std::make_pair(arg.first, *arg.second));
    }
}

template <typename First, typename... Rest>
void _component_to_xml(xml_node * c, First arg, Rest... args) {
	_component_to_xml(c, arg);
	_component_to_xml(c, args...);
}

} // End serial sub-namespace

template<typename... T>
inline void component_to_xml(xml_node * c, const T... args) {
	serial::_component_to_xml(c, args...);
}

/* Binary file helpers */

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
