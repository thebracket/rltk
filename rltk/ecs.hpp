#pragma once

#include <bitset>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <queue>
#include "serialization_utils.hpp"

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
	component_t() {
		data = C{};
		family();
	}
	component_t(C comp) : data(comp) {
		family();
	}
	std::size_t family_id;
	C data;

	inline void family() {
		static std::size_t family_id_tmp = base_component_t::type_counter++;
		family_id = family_id_tmp;
	}

	inline void save(std::ostream &lbfile) {
		data.save(lbfile);
	}
};

/*
 * Base class for the component store. Concrete component stores derive from this.
 */
struct base_component_store {
	virtual void erase_by_entity_id(const std::size_t &id)=0;
	virtual void really_delete()=0;
	virtual void save(std::ostream &lbfile)=0;
	virtual std::size_t size()=0;
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
		for (auto &item : components) {
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

	virtual void save(std::ostream &lbfile) override {
		for (auto &item : components) {
			serialize(lbfile, item.data.serialization_identity);
			serialize(lbfile, item.entity_id);
			item.save(lbfile);
		}
	}

	virtual std::size_t size() {
		return components.size();
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
			component_store.resize(temp.family_id+1);
		}
		if (!component_store[temp.family_id]) component_store[temp.family_id] = std::move(std::make_unique<component_store_t<component_t<C>>>());

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
inline void each(std::function<void(entity_t &)> &&func) {
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (!it->second.deleted) {
			func(it->second);
		}
	}
}

template <typename... Cs, typename F>
inline void each(F&& callback) {
	std::array<size_t, sizeof...(Cs)> family_ids{ component_t<Cs>{}.family_id... };
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (!it->second.deleted) {
			bool matches = true;
			for (const std::size_t &compare : family_ids) {
				if (!it->second.component_mask.test(compare)) {
					matches = false;
					break;
				}
			}
			if (matches) {
				// Call the functor
				callback(it->second, *it->second.component<Cs>()...);
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
				if (store) store->erase_by_entity_id(it->second.id);
			}
			entities_to_delete.insert(it->second.id);
		}
	}

	// Actually delete entities
	for (const std::size_t &id : entities_to_delete) entity_store.erase(id);

	// Now we erase components
	for (std::unique_ptr<base_component_store> &store : component_store) {
		if (store) store->really_delete();
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
	message_t() {
		C empty;
		data = empty;
		family();
	}
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

struct subscription_base_t {
};

struct base_system; // Forward declaration to avoid circles.

template <class C>
struct subscription_holder_t : subscription_base_t {
	std::vector<std::tuple<bool,std::function<void(C& message)>,base_system *>> subscriptions;
};

extern std::vector<std::unique_ptr<subscription_base_t>> pubsub_holder;

/* Base class for subscription mailboxes */
struct subscription_mailbox_t {
};

template <class C>
struct mailbox_t : subscription_mailbox_t {
	std::queue<C> messages;
};

/*
 * Systems should inherit from this class.
 */
struct base_system {
	virtual void configure() {}
	virtual void update(const double duration_ms)=0;
	std::string system_name = "Unnamed System";
	std::unordered_map<std::size_t, std::unique_ptr<subscription_mailbox_t>> mailboxes;
	
	template<class MSG>
	void subscribe(std::function<void(MSG &message)> &&destination) {
		MSG empty_message{};
		message_t<MSG> handle(empty_message);
		if (pubsub_holder.size() < handle.family_id + 1) {
			pubsub_holder.resize(handle.family_id + 1);
		}
		if (!pubsub_holder[handle.family_id]) {
			pubsub_holder[handle.family_id] = std::move(std::make_unique<subscription_holder_t<MSG>>());
		}
		static_cast<subscription_holder_t<MSG> *>(pubsub_holder[handle.family_id].get())->subscriptions.push_back(std::make_tuple(true,destination,nullptr));
	}

	template<class MSG>
	void subscribe_mbox() {
		MSG empty_message{};
		message_t<MSG> handle(empty_message);
		if (pubsub_holder.size() < handle.family_id + 1) {
			pubsub_holder.resize(handle.family_id + 1);			
		}
		if (!pubsub_holder[handle.family_id]) {
			pubsub_holder[handle.family_id] = std::move(std::make_unique<subscription_holder_t<MSG>>());
		}
		std::function<void(MSG &message)> destination; // Deliberately empty
		static_cast<subscription_holder_t<MSG> *>(pubsub_holder[handle.family_id].get())->subscriptions.push_back(std::make_tuple(false,destination,this));
		mailboxes[handle.family_id] = std::make_unique<mailbox_t<MSG>>();
	}

	template<class MSG>
	std::queue<MSG> * mbox() {
		message_t<MSG> handle(MSG{});
		auto finder = mailboxes.find(handle.family_id);
		if (finder != mailboxes.end()) {
			return &static_cast<mailbox_t<MSG> *>(finder->second.get())->messages;
		} else {
			return nullptr;
		}
	}
};

template <class MSG>
inline void emit(MSG &&message) {
	message_t<MSG> handle(message);
	if (pubsub_holder.size() > handle.family_id) {
		for (auto &func : static_cast<subscription_holder_t<MSG> *>(pubsub_holder[handle.family_id].get())->subscriptions) {
			if (std::get<0>(func) && std::get<1>(func)) {
				std::get<1>(func)(message);
			} else {
				// It is destined for the system's mailbox queue.
				auto finder = std::get<2>(func)->mailboxes.find(handle.family_id);
				if (finder != std::get<2>(func)->mailboxes.end()) {
					static_cast<mailbox_t<MSG> *>(finder->second.get())->messages.push(message);
				}
			}
		}
	}
}

extern std::vector<std::unique_ptr<base_system>> system_store;

struct system_profiling_t {
	double last = 0.0;
	double best = 1000000.0;
	double worst = 0.0;
};

extern std::vector<system_profiling_t> system_profiling;

template<typename S, typename ...Args>
inline void add_system( Args && ... args ) {
	system_store.push_back(std::make_unique<S>( std::forward<Args>(args) ... ));
	system_profiling.push_back(system_profiling_t{});
}

inline void ecs_configure() {
	for (std::unique_ptr<base_system> & sys : system_store) {
		sys->configure();
	}
}

inline void ecs_tick(const double duration_ms) {
	std::size_t count = 0;
	for (std::unique_ptr<base_system> & sys : system_store) {
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
		sys->update(duration_ms);
		std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

		system_profiling[count].last = duration;
		if (duration > system_profiling[count].worst) system_profiling[count].worst = duration;
		if (duration < system_profiling[count].best) system_profiling[count].best = duration;
		++count;
	}
	ecs_garbage_collect();
}

inline void ecs_save(std::ostream &lbfile) {
	// Store the number of entities and their ID numbers
	serialize(lbfile, entity_store.size());
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		serialize(lbfile, it->first);
	}

	// Store the last entity number
	serialize(lbfile, entity_t::entity_counter);

	// For each component type
	std::size_t number_of_components = 0;
	for (auto &it : component_store) {
		number_of_components += it->size();
	}
	serialize(lbfile, number_of_components);
	for (auto &it : component_store) {
		it->save(lbfile);
	}
}

inline void ecs_load(std::istream &lbfile, std::function<void(std::istream&,std::size_t,std::size_t)> helper) {
	entity_store.clear();
	component_store.clear();

	std::size_t number_of_entities;
	deserialize(lbfile, number_of_entities);
	for (std::size_t i=0; i<number_of_entities; ++i) {
		std::size_t entity_id;
		deserialize(lbfile, entity_id);
		create_entity(entity_id);
	}
	deserialize(lbfile, entity_t::entity_counter);

	std::size_t number_of_components;
	deserialize(lbfile, number_of_components);
	for (std::size_t i=0; i<number_of_components; ++i) {
		std::size_t serialization_identity;
		std::size_t entity_id;
		deserialize(lbfile, serialization_identity);
		deserialize(lbfile, entity_id);
		helper(lbfile, serialization_identity, entity_id);
	}
}

inline std::string ecs_profile_dump() {
	std::stringstream ss;
	ss.precision(3);
	ss << std::fixed;
	ss << "SYSTEMS PERFORMANCE IN MICROSECONDS:\n";
	ss << std::setw(20) << "System" << std::setw(20) << "Last" << std::setw(20) << "Best" << std::setw(20) << "Worst\n";
	for (std::size_t i=0; i<system_profiling.size(); ++i) {
		ss << std::setw(20) << system_store[i]->system_name 
			<< std::setw(20) << system_profiling[i].last 
			<< std::setw(20) << system_profiling[i].best 
			<< std::setw(20) << system_profiling[i].worst << "\n";
	}
	return ss.str();
}

}
