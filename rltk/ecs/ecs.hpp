#pragma once

#include <bitset>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>

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
	bool deleted = false;
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
	virtual void erase_by_entity_id(const std::size_t &id)=0;
	virtual void really_delete()=0;
};

// Forward declaration
void unset_component_mask(const std::size_t id, const std::size_t family_id, bool delete_if_empty=true);

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
	
	virtual void erase_by_entity_id(const std::size_t &id) override {
		for (auto item : components) {
			if (item.entity_id == id) {
				item.deleted=true;
				unset_component_mask(id, item.family_id);
			}
		}
	}

	virtual void really_delete() {
		components.erase(std::remove_if(components.begin(), components.end(),
			[] (auto x) { return x.deleted; }), 
			components.end());
	}
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
		id = ID;
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

	bool deleted = false;

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
		if (deleted) throw std::runtime_error("Cannot assign to a deleted entity");
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
		if (deleted) throw std::runtime_error("Entity is deleted");
		C empty_component;
		component_t<C> temp(empty_component);
		if (!component_mask.test(temp.family_id)) throw std::runtime_error("Entity #" + std::to_string(id) + " does not have the requested component.");
		for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
			if (component.entity_id == id) return &component.data;
		}
		return nullptr;
	}
};

/*
 * The actual storage of entities. Right now, it's a map to enable quick reference to an entity by ID.
 */
extern std::unordered_map<std::size_t, entity_t> entity_store;

/*
 * Remove a reference to a component in an entity's bitmask
 */
inline void unset_component_mask(const std::size_t id, const std::size_t family_id, bool delete_if_empty) {
	auto finder = entity_store.find(id);
	if (finder != entity_store.end()) {
		finder->second.component_mask.reset(family_id);
		if (delete_if_empty && finder->second.component_mask.none()) finder->second.deleted = true;
	}
}

/*
 * entity(ID) is used to reference an entity. So you can, for example, do:
 * entity(12)->component<position_component>()->x = 3;
 */
inline entity_t * entity(const std::size_t id) {
	auto finder = entity_store.find(id);
	if (finder == entity_store.end()) throw std::runtime_error("Entity with ID " + std::to_string(id) + " does not exist");
	if (finder->second.deleted) throw std::runtime_error("Entity #" + std::to_string(id) + " is deleted");
	return &finder->second;
}

/*
 * Creates an entity with a new ID #. Returns a pointer to the entity, to enable
 * call chaining. For example create_entity()->assign(foo)->assign(bar)
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
 * vector of pointers to the entities. It does not check for component deletion.
 */
template<class C>
inline std::vector<entity_t *> entities_with_component() {
	C empty_component;
	std::vector<entity_t *> result;
	component_t<C> temp(empty_component);
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (!it->second.deleted && it->second.component_mask.test(temp.family_id)) {
			result.push_back(&it->second);
		}
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
		entity_t e = *entity(component.entity_id);
		if (!e.deleted && !component.deleted) {
			func(e, component.data);
		}
	}
}

/*
 * each, overloaded with a function/lambda that accepts an entity, will call the provided
 * function on _every_ entity in the system.
 */
inline void each(std::function<void(entity_t &)> func) {
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (!it->second.deleted) {
			func(it->second);
		}
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
		if (!it->second.deleted && it->second.component_mask.test(temp.family_id)) {
			for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
				if (it->second.id == component.entity_id && !component.deleted) {
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
		if (!it->second.deleted && it->second.component_mask.test(temp.family_id) && it->second.component_mask.test(temp2.family_id)) {
			for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
				if (it->second.id == component.entity_id && !component.deleted) {
					for (component_t<C2> &component2 : static_cast<component_store_t<component_t<C2>> *>(component_store[temp2.family_id].get())->components) {
						if (it->second.id == component2.entity_id && !component2.deleted) {
							func(it->second, component.data, component2.data);
						}
					}
				}
			}
		}
	}
}

/*
 * Marks an entity (specified by ID#) as deleted.
 */
inline void delete_entity(const std::size_t id) {
	entity(id)->deleted = true;
}

/*
 * Marks an entity as deleted.
 */
inline void delete_entity(entity_t &e) {
	e.deleted = true;
}

/*
 * Marks an entity's component as deleted.
 */
template<class C>
inline void delete_component(const std::size_t entity_id, bool delete_entity_if_empty=false) {
	entity_t e = *entity(entity_id);
	C empty_component;
	component_t<C> temp(empty_component);
	if (!e.component_mask.test(temp.family_id)) throw std::runtime_error("Entity #" + std::to_string(entity_id) + " does not have a component to delete.");
	for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
		if (component.entity_id == entity_id) {
			component.deleted = true;
			unset_component_mask(entity_id, temp.family_id, delete_entity_if_empty);
		}
	}
}

/*
 * This should be called periodically to actually erase all entities and components that are marked as deleted.
 */
inline void ecs_garbage_collect() {
	std::unordered_set<std::size_t> entities_to_delete;

	// Ensure that components are marked as deleted, and list out entities for erasure
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (it->second.deleted) {
			for (std::unique_ptr<base_component_store> &store : component_store) {
				store->erase_by_entity_id(it->second.id);
			}
			entities_to_delete.insert(it->second.id);
		}
	}

	// Actually delete entities
	for (const std::size_t &id : entities_to_delete) entity_store.erase(id);

	// Now we erase components
	for (std::unique_ptr<base_component_store> &store : component_store) {
		store->really_delete();
	}
}

/*
 * Base class from which all messages must derive.
 */
struct base_message_t {
	static std::size_t type_counter;
};

/*
 * Handle class for messages
 */
template<class C>
struct message_t : public base_message_t {
	message_t(C comp) : data(comp) {
		family();
	}
	std::size_t family_id;
	C data;

	inline void family() {
		static std::size_t family_id_tmp = base_message_t::type_counter++;
		family_id = family_id_tmp;
	}
};

template <class C>
struct pub_sub_handler {
	std::function<void(C&message)> destination;
};

struct subscription_base_t {
};

template <class C>
struct subscription_holder_t : subscription_base_t {
	std::vector<std::function<void(C&message)>> subscriptions;
};

extern std::vector<std::unique_ptr<subscription_base_t>> pubsub_holder;

/*
 * Systems should inherit from this class.
 */
struct base_system {
	virtual void configure() {}
	virtual void update(const double duration_ms)=0;
	
	template<class MSG>
	void subscribe(std::function<void(MSG &message)> destination) {
		MSG empty_message{};
		message_t<MSG> handle(empty_message);
		if (pubsub_holder.size() < handle.family_id + 1) {
			pubsub_holder.resize(handle.family_id + 1);
			pubsub_holder[handle.family_id] = std::move(std::make_unique<subscription_holder_t<MSG>>());
		}
		static_cast<subscription_holder_t<MSG> *>(pubsub_holder[handle.family_id].get())->subscriptions.push_back(destination);
	}

	template <class MSG>
	void emit(MSG &&message) {
		message_t<MSG> handle(message);
		for (auto &func : static_cast<subscription_holder_t<MSG> *>(pubsub_holder[handle.family_id].get())->subscriptions) {
			func(message);
		}
	}
};

extern std::vector<std::unique_ptr<base_system>> system_store;

template<typename S, typename ...Args>
inline void add_system( Args && ... args ) {
	system_store.push_back(std::make_unique<S>( std::forward<Args>(args) ... ));
}

inline void ecs_configure() {
	for (std::unique_ptr<base_system> & sys : system_store) {
		sys->configure();
	}
}

inline void ecs_tick(const double duration_ms) {
	for (std::unique_ptr<base_system> & sys : system_store) {
		sys->update(duration_ms);
	}
	ecs_garbage_collect();
}

}
