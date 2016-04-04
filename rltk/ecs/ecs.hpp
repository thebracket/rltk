#pragma once

#include <bitset>
#include <vector>
#include <iostream>

namespace rltk {

constexpr std::size_t MAX_COMPONENTS = 128;

struct entity_id {
	std::size_t id;
	bool operator == (const entity_id &other) const { return other.id == id; }
	bool operator != (const entity_id &other) const { return other.id != id; }
};

struct base_component_t {
	static std::size_t type_counter;
};

template<class C>
struct component_t : public base_component_t {
	component_t(C comp) : data(comp) {
		family();
	}
	std::size_t family_id;
	C data;

	inline void family() {
		static std::size_t family_id_tmp = base_component_t::type_counter++;
		family_id = family_id_tmp;
	}
};

struct base_component_store {	
};

template<class C>
struct component_store_t : public base_component_store {
	std::vector<C> components;
};

extern std::vector<std::unique_ptr<base_component_store>> component_store;

struct entity_t {
	entity_t() {
		id = {entity_t::entity_counter++};
	}
	entity_t(const std::size_t ID) {
		entity_t::entity_counter = ID+1;
		id = {ID};
	}

	static std::size_t entity_counter;
	entity_id id;

	bool operator == (const entity_t &other) const { return other.id == id; }
	bool operator != (const entity_t &other) const { return other.id != id; }

	std::bitset<MAX_COMPONENTS> component_mask;

	template<class C>
	inline void assign(C &&component) {
		component_t<C> temp(component);
		if (component_store.size() < temp.family_id+1) {
			component_store.push_back(std::make_unique<component_store_t<component_t<C>>>());
		}
		static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components.push_back(temp);
		component_mask.set(temp.family_id);
		std::cout << component_mask << "\n";
		std::cout << component_store.size() << " - " << static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components.size() << "\n";
	}
};

}
