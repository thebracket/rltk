#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <memory>
#include <sstream>
#include <cstdio>
#include "color_t.hpp"
#include "vchar.hpp"
#include "filesystem.hpp"

namespace rltk {

template<typename T>
inline T from_string(const std::string &val) {
    std::stringstream ss;
    ss << val;
    T result;
    ss >> result;
    return result;
}
template<>
inline uint8_t from_string(const std::string &val) {
    std::stringstream ss;
    ss << val;
    int result;
    ss >> result;
    return static_cast<uint8_t>(result);
}
template<>
inline std::string from_string(const std::string &val) {
    return val;
}

struct xml_node {
    xml_node() {};
    xml_node(const std::string &Name) : name(Name) {}
    xml_node(const std::string &Name, const int Depth) : name(Name), depth(Depth) {}

    xml_node * add_node(const std::string &name);
    void add_node(xml_node x);
    std::string indent() const;
    void save(std::ofstream &lbfile) const;
    void dump(std::stringstream &lbfile) const;
    std::string dump() const;
    void add_value(const std::string &key, const std::string &val);

    std::string name = "";

    std::size_t count() {
        std::size_t count = 0;
        for (xml_node &node : children) {
            node.count(count);
        }
        return count;
    }

    void count(std::size_t &count) {
        ++count;
        for (xml_node &node : children) {
            node.count(count);
        }
    }

    xml_node * find(const std::string name) {
        for (xml_node &node : children) {
            if (node.name == name) return &node;
        }
        return nullptr;
    }

    template<typename T>
    T val(const std::string &key) {
        for (const auto &val : values) {
            if (val.first == key) {
                return from_string<T>(val.second);
            }
        }
        throw std::runtime_error(std::string("Key not found:") + key);
    }

    inline void iterate_child(const std::string &name, const std::function<void(xml_node *)> &func) {
        xml_node * vec = find(name);
        for (xml_node child : vec->children) {
            func(&child);
        }
    }

    inline color_t color(const std::string &name) {
        xml_node * c = find(name);
        return color_t{c->val<uint8_t>("r"), c->val<uint8_t>("g"), c->val<uint8_t>("b")};
    }

    inline rltk::vchar vchar() {
        rltk::vchar c;
        c.glyph = val<uint32_t>("glyph");
        c.foreground = color("foreground");
        c.background = color("background");
        return c;
    }

    const int depth = 0;
    std::vector<xml_node> children;
    std::vector<std::pair<std::string, std::string>> values;

};

struct xml_writer {
    xml_writer(const std::string &fn, const std::string &root_name) : filename(fn), root(xml_node(root_name,0)) {
        if (exists(filename)) std::remove(filename.c_str());
	    lbfile = std::make_unique<std::ofstream>(filename, std::ios::out | std::ios::binary);
    }

    xml_writer(std::unique_ptr<std::ofstream> &&f, const std::string &root_name) : lbfile(std::move(f)), root(xml_node(root_name,0)) {}

    inline void commit() {        
        root.save(*lbfile);
    }

    inline xml_node * add_node(const std::string &name) {
        return root.add_node(name);
    }

private:
    std::unique_ptr<std::ofstream> lbfile;
    const std::string filename = "";
    xml_node root;
};

struct xml_reader {
    xml_reader(const std::string &fn) : filename(fn) {
        if (!exists(filename)) throw std::runtime_error(std::string("File not found: ") + filename);
	    lbfile = std::make_unique<std::ifstream>(filename, std::ios::in | std::ios::binary);
        load();
    }

    xml_reader(std::unique_ptr<std::ifstream> &&f) : lbfile(std::move(f)) {
        load();
    }

    inline xml_node * get() { return &root; }

private:
    std::unique_ptr<std::ifstream> lbfile;
    const std::string filename = "";
    xml_node root;

    void load();
};

}
