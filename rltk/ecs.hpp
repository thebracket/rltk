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
#include <future>
#include <mutex>
#include <boost/optional.hpp>
#include <typeinfo>
#include "serialization_utils.hpp"
#include "xml.hpp"

namespace rltk {

/*
 * Constant defining the maximum number of components to support. This sizes the bitsets,
 * so we don't want it to be much bigger than needed.
 */
constexpr std::size_t MAX_COMPONENTS = 64;

/*
 * If the current component set does not support serialization, this will become true.
 */
 static bool ecs_supports_serialization = true;

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

/* Extracts xml_identity from a class, or uses the RTTI name if one isn't available */
template< class T >
struct has_to_xml_identity
{
    typedef char(&YesType)[1];
    typedef char(&NoType)[2];
    template< class, class > struct Sfinae;

    template< class T2 > static YesType Test( Sfinae<T2, decltype(std::declval<T2>().xml_identity)> *);
    template< class T2 > static NoType  Test( ... );
    static const bool value = sizeof(Test<T>(0))==sizeof(YesType);
};

template <typename T>
struct _calc_xml_identity {

	template<class Q = T>
	typename std::enable_if< has_to_xml_identity<Q>::value, void >::type
    test(T &data, std::string &id)
    {
		id = data.xml_identity;
    }

	template<class Q = T>
	typename std::enable_if< !has_to_xml_identity<Q>::value, void >::type
    test(T &data, std::string &id)
    {
		id = typeid(data).name();
    }
};

template<class T>
struct _ecs_check_for_to_xml
{
    template<class Q = T>
	typename std::enable_if< serial::has_to_xml_method<Q>::value, void >::type
    test(xml_node * c, T &data)
    {
		data.to_xml(c);
    }

    template<class Q = T>
	typename std::enable_if< !serial::has_to_xml_method<Q>::value, void >::type
    test(xml_node * c, T &data)
    {
        ecs_supports_serialization = false;
    }
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

	inline std::string xml_identity() {
		std::string id;
		_calc_xml_identity<C>().test(data, id);
		return id;
	}

	inline void to_xml(xml_node * c) {
		_ecs_check_for_to_xml<C> serial;
		serial.test(c, data);
	}
};

/*
 * Base class for the component store. Concrete component stores derive from this.
 */
struct base_component_store {
	virtual void erase_by_entity_id(const std::size_t &id)=0;
	virtual void really_delete()=0;
	virtual void save(xml_node * xml)=0;
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
	
	virtual void erase_by_entity_id(const std::size_t &id) override final {
		for (auto &item : components) {
			if (item.entity_id == id) {
				item.deleted=true;
				unset_component_mask(id, item.family_id);
			}
		}
	}

	virtual void really_delete() override final {
		components.erase(std::remove_if(components.begin(), components.end(),
			[] (auto x) { return x.deleted; }), 
			components.end());
	}

	virtual void save(xml_node * xml) override final {
		for (auto &item : components) {
			xml_node * body = xml->add_node(item.xml_identity());			
			item.to_xml(body);
			body->add_value("entity_id", rltk::serial::to_string(item.entity_id));
		}
	}

	virtual std::size_t size() override final {
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
	inline boost::optional<C&> component() noexcept {
		boost::optional<C&> result;
		if (deleted) return result;

		C empty_component;
		component_t<C> temp(empty_component);
		if (!component_mask.test(temp.family_id)) return result;
		for (component_t<C> &component : static_cast<component_store_t<component_t<C>> *>(component_store[temp.family_id].get())->components) {
			if (component.entity_id == id) {
				result = component.data;
				return result;
			}
		}
		return result;
	}
};

/*
 * The actual storage of entities. Right now, it's a map to enable quick reference to an entity by ID.
 */
extern std::unordered_map<std::size_t, entity_t> entity_store;

/*
 * Remove a reference to a component in an entity's bitmask
 */
void unset_component_mask(const std::size_t id, const std::size_t family_id, bool delete_if_empty);

/*
 * entity(ID) is used to reference an entity. So you can, for example, do:
 * entity(12)->component<position_component>()->x = 3;
 */
boost::optional<entity_t&> entity(const std::size_t id) noexcept;

/*
 * Creates an entity with a new ID #. Returns a pointer to the entity, to enable
 * call chaining. For example create_entity()->assign(foo)->assign(bar)
 */
boost::optional<entity_t&> create_entity();

/*
 * Creates an entity with a specified ID #. You generally only do this during loading.
 */
boost::optional<entity_t&> create_entity(const std::size_t new_id);

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
void each(std::function<void(entity_t &)> &&func);

/*
 * Variadic each. Use this to call a function for all entities having a discrete set of components. For example,
 * each<position, ai>([] (entity_t &e, position &pos, ai &brain) { ... code ... });
 */
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

// Forward declaration
struct base_message_t;

/*
 * Parallel variadic each. Use this to call a function for all entities having a discrete set of components. For example,
 * each<position, ai>([] (entity_t &e, position &pos, ai &brain) { ... code ... });
 * Each function will be called in parallel, and the function returns when every call is done.
 * 
 * Be careful: not every platform uses a proper thread pool, so this can end up with a thundering herd of threads
 * competing for CPU time! Minimal effort is made to provide internal thread-safety at this time - so it's up to you to
 * handle that for now.
 *
 * WARNING: It's really easy to shoot yourself in the foot with this function. Be aware of synchronization, race conditions,
 * etc. If you aren't sure, don't use it!
 */
template <typename... Cs, typename F>
inline void parallel_each(F&& callback) {
	#if defined(_OPENMP)
	std::vector<std::function<void()>> callbacks;
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
				callbacks.emplace_back([it, &callback] () { callback(it->second, *it->second.component<Cs>()...); });
			}
		}
	}

	#pragma omp parallel for
	for (auto i=0; i<callbacks.size(); ++i) {
		const std::function<void()> &cb = callbacks.at(i);
		cb();
	}
	#else	
	std::vector<std::future<void>> futures;
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
				futures.emplace_back(std::async(std::launch::async, [it, &callback] () { callback(it->second, *it->second.component<Cs>()...); }));
			}
		}
	}
	for (auto &f : futures) {
		f.wait();
	}
	#endif
}

/*
 * Marks an entity (specified by ID#) as deleted.
 */
void delete_entity(const std::size_t id) noexcept;

/*
 * Marks an entity as deleted.
 */
void delete_entity(entity_t &e) noexcept;

/*
 * Deletes all entities
 */
void delete_all_entities() noexcept;

/*
 * Marks an entity's component as deleted.
 */
template<class C>
inline void delete_component(const std::size_t entity_id, bool delete_entity_if_empty=false) noexcept {
	auto eptr = entity(entity_id);
	if (!eptr) return;
	entity_t e = *entity(entity_id);
	C empty_component;
	component_t<C> temp(empty_component);
	if (!e.component_mask.test(temp.family_id)) return;
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
void ecs_garbage_collect();

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
	virtual void deliver_messages()=0;
};

struct base_system; // Forward declaration to avoid circles.

/* Base class for subscription mailboxes */
struct subscription_mailbox_t {
};

template <class C>
struct mailbox_t : subscription_mailbox_t {
	std::queue<C> messages;
};

template <class C>
struct subscription_holder_t : subscription_base_t {
	std::queue<C> delivery_queue;	
	std::mutex delivery_mutex;
	std::vector<std::tuple<bool,std::function<void(C& message)>,base_system *>> subscriptions;

	virtual void deliver_messages() override {
		std::lock_guard<std::mutex> guard(delivery_mutex);
		while (!delivery_queue.empty()) {
			C message = delivery_queue.front();
			delivery_queue.pop();
			message_t<C> handle(message);

			for (auto &func : subscriptions) {
				if (std::get<0>(func) && std::get<1>(func)) {
					std::get<1>(func)(message);
				} else {
					// It is destined for the system's mailbox queue.
					auto finder = std::get<2>(func)->mailboxes.find(handle.family_id);
					if (finder != std::get<2>(func)->mailboxes.end()) {
						static_cast<mailbox_t<C> *>(finder->second.get())->messages.push(message);
					}
				}
			}
		}
	}
};

extern std::vector<std::unique_ptr<subscription_base_t>> pubsub_holder;

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

/*
 * Submits a message for delivery. It will be delivered to every system that has issued a subscribe or subscribe_mbox
 * call.
 */
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

/*
 * Submits a message for delivery. It will be delivered to every system that has issued a subscribe or subscribe_mbox
 * call at the end of the next system execution. This is thead-safe, so you can emit_defer from within a parallel_each.
 */
template <class MSG>
inline void emit_deferred(MSG &&message) {
	message_t<MSG> handle(message);
	if (pubsub_holder.size() > handle.family_id) {

		auto * subholder = static_cast<subscription_holder_t<MSG> *>(pubsub_holder[handle.family_id].get());
		std::lock_guard<std::mutex> postlock(subholder->delivery_mutex);
		subholder->delivery_queue.push(message);
	}
}

/* Delivers the queue; called at the end of each system call */
inline void deliver_messages() {
	for (auto &holder : pubsub_holder) {
		if (holder) holder->deliver_messages();
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

void delete_all_systems();

void ecs_configure();

void ecs_tick(const double duration_ms);

void ecs_save(std::unique_ptr<std::ofstream> lbfile);

void ecs_load(std::unique_ptr<std::ifstream> lbfile, const std::function<void(xml_node *, std::size_t, std::string)> &helper);

std::string ecs_profile_dump();

}
