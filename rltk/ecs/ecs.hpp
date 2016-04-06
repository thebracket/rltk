#pragma once

#include <bitset>
#include <vector>
#include <iostream>
#include <unordered_map>

namespace rltk {

/*
 * Constant defining the maximum number of components to support. This sizes the bitsets,
 * so we don't want it to be much bigger than needed.
 */
constexpr std::size_t MAX_COMPONENTS = 64;

/*
 * Base type for component handles. Exists so that we can have a vector of pointers to
 * derived classes. entity_id is included to allow a quick reference without a static cast.
 * type_counter is used as a static member, referenced from component_t - the handle class.
 */
struct base_component_t {
	static std::size_t type_counter;
	std::size_t entity_id;
};

/*
 * component_t is a handle class for components. It inherits from base_component, allowing
 * the component store to have vectors of base_component_t *, where each type is a concrete
 * specialized class containing the component data. It does some magic with a static type_counter
 * to ensure that each instance with the same template type will have a unique family_id - this is
 * then used to reference the correct component store.
 */
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

/*
 * Base class for the component store. Concrete component stores derive from this.
 */
struct base_component_store {	
};

/*
 * Component stores are just a vector of type C (the component). They inherit from
 * base_component_store, to allow for a vector of base_component_store*, with each
 * casting to a concrete vector of that type. The types are indexed by the family_id
 * created for a type with component_t<C>. This guarantees that each component type
 * is stored in a big contiguous vector, with only one de-reference required to find
 * the right store.
 */
template<class C>
struct component_store_t : public base_component_store {
	std::vector<C> components;
};

/*
 * The actual component store, a vector of pointers to component stores for each
 * known type.
 */
extern std::vector<std::unique_ptr<base_component_store>> component_store;

/*
 * All entities are of type entity_t. They should be created with create_entity (below).
 */
struct entity_t {

	/*
	 * Default constructor - use the next available entity id #.
	 */
	entity_t() {
		id = {entity_t::entity_counter++};
	}

	/*
	 * Construct with a specified entity #. Moves the next available entity # to this id+1.
	 */
	entity_t(const std::size_t ID) {
		entity_t::entity_counter = ID+1;
		id = {ID};
	}

	/*
	 * Static ID counter - used to ensure that entity IDs are unique. This would need to be atomic
	 * in a threaded app.
	 */
	static std::size_t entity_counter;

	/*
	 * The entities ID number. Used to identify the entity. These should be unique.
	 */
	std::size_t id;

	/*
	 * Overload == and != to allow entities to be compared for likeness.
	 */
	bool operator == (const entity_t &other) const { return other.id == id; }
	bool operator != (const entity_t &other) const { return other.id != id; }

	/*
	 * A bitset storing whether or not an entity has each component type. These are set with the family_id
	 * determined in the component_t system above.
	 */
	std::bitset<MAX_COMPONENTS> component_mask;

	/*
	 * Assign a component to this entity. Determines the family_id of the component type, sets the bitmask to
	 * include the component, marks the component as belonging to the entity, and puts it in the appropriate
	 * component store.
	 */
	template<class C>
	inline entity_t * assign(C &&component) {
		component_t<C> temp(component);
		temp.entity_id = id;
		if (component_store.size() < temp.family_id+1) {
			component_store.push_back(std::make_unique<component_store_t<component_t<C>>>());
		}
		static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components.push_back(temp);
		component_mask.set(temp.family_id);
		return this;
	}

	/*
	 * Find a component of the specified type that belongs to the entity.
	 */
	template <class C>
	inline C * component() {
		C empty_component;
		component_t<C> temp(empty_component);
		if (!component_mask.test(temp.family_id)) throw std::runtime_error("Entity #" + std::to_string(id) + " does not have the requested component.");
		for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
			if (component.entity_id == id) return &component.data;
		}
	}
};

/*
 * The actual storage of entities. Right now, it's a map to enable quick reference to an entity by ID.
 */
extern std::unordered_map<std::size_t, entity_t> entity_store;

/*
 * entity(ID) is used to reference an entity. So you can, for example, do:
 * entity(12)->component<position_component>()->x = 3;
 */
inline entity_t * entity(std::size_t id) {
	auto finder = entity_store.find(id);
	if (finder == entity_store.end()) throw std::runtime_error("Entity with ID " + std::to_string(id) + " does not exist");
	return &finder->second;
}

/*
 * Creates an entity with a new ID #.
 */
inline entity_t * create_entity() {
	entity_t new_entity;
	entity_store.emplace(new_entity.id, new_entity);
	return entity(new_entity.id);
}

/*
 * Creates an entity with a specified ID #. You generally only do this during loading.
 */
inline entity_t * create_entity(const std::size_t new_id) {
	entity_t new_entity(new_id);
	entity_store.emplace(new_entity.id, new_entity);
	return entity(new_entity.id);
}

/*
 * Finds all entities that have a component of the type specified, and returns a
 * vector of pointers to the entities.
 */
template<class C>
inline std::vector<entity_t *> entities_with_component() {
	C empty_component;
	std::vector<entity_t *> result;
	component_t<C> temp(empty_component);
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (it->second.component_mask.test(temp.family_id)) result.push_back(&it->second);
	}
	return result;
}

/*
 * all_components takes a component type, and calls the provided function/lambda on
 * every component, alongside it's owning entity. For example,
 * all_components<position>([] (entity_t &e, position &p) {...}) would execute the
 * function body (...) for every entity/component position pair.
 */
template <class C>
inline void all_components(typename std::function<void(entity_t &, C &)> func) {
	C empty_component;
	component_t<C> temp(empty_component);
	for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
		func(*entity(component.entity_id), component.data);
	}
}

/*
 * each, overloaded with a function/lambda that accepts an entity, will call the provided
 * function on _every_ entity in the system.
 */
inline void each(std::function<void(entity_t &)> func) {
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		func(it->second);
	}
}

/*
 * each, overloaded with a component class and a function that accepts an entity_id and a
 * function/lambda that accepts an entity/component combination calls the provided function
 * on each entity/component that matches. This is very similar to all_components, but is
 * entity-first: it cares about the bitmask. all_components is probably faster.
 */
template <class C>
inline void each(typename std::function<void(entity_t &, C &)> func) {
	C empty_component;
	component_t<C> temp(empty_component);
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (it->second.component_mask.test(temp.family_id)) {
			for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
				if (it->second.id == component.entity_id) {
					func(it->second, component.data);
				}
			}
		}
	}
}

/*
 * each, overloaded with two classes, calls-back for entities that have BOTH component types,
 * calling with the entity and both components.
 */
template <class C, class C2>
inline void each(typename std::function<void(entity_t &, C &, C2 & )> func) {
	C empty_component;
	C2 empty_component2;
	component_t<C> temp(empty_component);
	component_t<C2> temp2(empty_component2);

	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (it->second.component_mask.test(temp.family_id) && it->second.component_mask.test(temp2.family_id)) {
			for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
				if (it->second.id == component.entity_id) {
					for (component_t<C2> &component2 : static_cast<component_store_t<component_t<C2>> *>(component_store[temp2.family_id].get())->components) {
						if (it->second.id == component2.entity_id) {
							func(it->second, component.data, component2.data);
						}
					}
				}
			}
		}
	}
}

}
