#include "xml.hpp"
#include <stack>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <iostream>
#include <string>

using namespace std::string_literals;

namespace rltk {

xml_node * xml_node::add_node(const std::string &name) {
    children.push_back(xml_node(name, depth+1));
    return &children[children.size()-1];
}

void xml_node::add_node(xml_node x) {
    children.push_back(x);
}

std::string xml_node::indent() const {
    std::string result;
    for (int i=0; i<depth; ++i) result += ' ';
    return result;
}

void xml_node::save(std::ofstream &lbfile) const {
    lbfile << indent() << "<" << name << ">\n";
    for (const auto & val : values) {
        lbfile << indent() << " <" << val.first << ":value>" << val.second << "</" << val.first << ":value>\n";
    }
    for (const xml_node &child : children) {
        child.save(lbfile);
    }
    lbfile << indent() << "</" << name << ">\n";
}

void xml_node::dump(std::stringstream &lbfile) const {
    lbfile << indent() << "<" << name << ">\n";
    for (const auto & val : values) {
        lbfile << indent() << " <" << val.first << ":value>" << val.second << "</" << val.first << ":value>\n";
    }
    for (const xml_node &child : children) {
        child.dump(lbfile);
    }
    lbfile << indent() << "</" << name << ">\n";    
}

std::string xml_node::dump() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

void xml_node::add_value(const std::string &key, const std::string &val) {
    values.emplace_back(std::make_pair(key, val));
}

static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

static inline void erase_all(std::string &s, const std::string &to_remove) {
    auto pos = s.find(to_remove);
    while (pos != std::string::npos) {
        s.erase(pos, to_remove.size());
	pos = s.find(to_remove);
    }
}

static inline std::string remove_xml_braces(std::string s) {
    erase_all(s, "<");
    erase_all(s, ">");
    return s;
}

static inline std::string remove_colon_value(std::string s) {
    erase_all(s, ":value");
    return s;
}

void xml_reader::load() {
    std::stack<xml_node> stack;

    bool first_line = true;
    std::string line;
    while (getline(*lbfile, line)) {
        std::string trimmed = ltrim(line);
        erase_all(trimmed, "\n");

        if (first_line) {
            root.name = remove_xml_braces(trimmed);
            first_line = false;
        } else if (trimmed == "</"s + root.name + ">"s) {
            // We're at the end.
        } else if (stack.empty()) {
            // We're at the beginning, so we need to create a node.
            stack.push(xml_node(remove_xml_braces(trimmed)));
        } else if (trimmed == "</"s + stack.top().name + ">"s ) {
            // We're at the end of the element
            if (stack.size() > 1) {
                xml_node current = stack.top();
                stack.pop();
                stack.top().add_node(current);
            } else {
                xml_node current = stack.top();
                stack.pop();
                root.add_node(current);
            }
        } else {
            // Are we looking at a new node or a value?
            if (trimmed.find(":value>")==std::string::npos) {
                // It's a new child
                stack.push(xml_node(remove_xml_braces(trimmed)));
            } else {
                // It's a value
                std::string key = remove_colon_value(remove_xml_braces(trimmed.substr(0,trimmed.find(">"))));
                std::string val = trimmed.substr(trimmed.find(">")+1);
                val = val.substr(0, val.find("<"));
                stack.top().add_value(key, val);
            }
        }
    }
}

}
