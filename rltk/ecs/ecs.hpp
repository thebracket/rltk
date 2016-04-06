#pragma once

#include <bitset>
#include <vector>
#include <iostream>
#include <unordered_map>

namespace rltk {

constexpr std::size_t MAX_COMPONENTS = 128;

struct entity_id {
	std::size_t id;
	bool operator == (const entity_id &other) const { return other.id == id; }
	bool operator != (const entity_id &other) const { return other.id != id; }
};

struct base_component_t {
	static std::size_t type_counter;
	std::size_t entity_id;
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
	inline entity_t * assign(C &&component) {
		component_t<C> temp(component);
		temp.entity_id = id.id;
		if (component_store.size() < temp.family_id+1) {
			component_store.push_back(std::make_unique<component_store_t<component_t<C>>>());
		}
		static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components.push_back(temp);
		component_mask.set(temp.family_id);
		return this;
	}
};

extern std::unordered_map<std::size_t, entity_t> entity_store;

inline entity_t * entity(std::size_t id) {
	auto finder = entity_store.find(id);
	if (finder == entity_store.end()) throw std::runtime_error("Entity with ID " + std::to_string(id) + " does not exist");
	return &finder->second;
}

inline entity_t * create_entity() {
	entity_t new_entity;
	entity_store.emplace(new_entity.id.id, new_entity);
	return entity(new_entity.id.id);
}

template<class C>
inline std::vector<entity_t *> entity_component(C &&empty_component) {
	std::vector<entity_t *> result;
	component_t<C> temp(empty_component);
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (it->second.component_mask.test(temp.family_id)) result.push_back(&it->second);
	}
	return result;
}

inline void each(std::function<void(entity_t &)> func) {
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		func(it->second);
	}
}

template <class C>
inline void all_components(typename std::function<void(entity_t &, C &)> func) {
	C empty_component;
	component_t<C> temp(empty_component);
	for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
		func(*entity(component.entity_id), component.data);
	}
}

template <class C>
inline void each(typename std::function<void(entity_t &, C &)> func) {
	C empty_component;
	component_t<C> temp(empty_component);
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (it->second.component_mask.test(temp.family_id)) {
			for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
				if (component.entity_id == it->second.id.id) {					
					func(it->second, component.data);
				}
			}
		}
	}
}

template <class C, class C2>
inline void each(typename std::function<void(entity_t &, C &, C2 & )> func) {
	C empty_component;
	C2 empty_component2;
	component_t<C> temp(empty_component);
	component_t<C2> temp2(empty_component2);

	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (it->second.component_mask.test(temp.family_id) && it->second.component_mask.test(temp2.family_id)) {
			for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
				if (component.entity_id == it->second.id.id) {					
					for (component_t<C2> &component2 : static_cast<component_store_t<component_t<C2>> *>(component_store[temp2.family_id].get())->components) {
						func(it->second, component.data, component2.data);
					}
				}
			}
		}
	}
}

}
