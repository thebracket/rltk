#pragma once

#include "../cereal/cereal.hpp"
#include <cereal/types/memory.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/vector.hpp>

namespace rltk {

    // Forward declarations
    class ecs;
    struct entity_t;
    struct base_system;
    extern ecs default_ecs;

    namespace impl {

        template<class C>
        inline void assign(ecs &ECS, entity_t &E, C component);

        template <class C>
        inline C * component(ecs &ECS, entity_t &E) noexcept;

        template<class MSG>
        inline void subscribe(ecs &ECS, base_system &B, std::function<void(MSG &message)> destination);

        template<class MSG>
        inline void subscribe_mbox(ecs &ECS, base_system &B);

        inline void unset_component_mask(ecs &ECS, const std::size_t id, const std::size_t family_id, bool delete_if_empty=false);
    }

    /*
     * Base class from which all messages must derive.
     */
    struct base_message_t {
        static std::size_t type_counter;
    };

    /* Class for storing profile data */
    struct system_profiling_t {
        double last = 0.0;
        double best = 1000000.0;
        double worst = 0.0;
    };

    struct base_system;

    namespace impl {

        /*
         * Constant defining the maximum number of components to support. This sizes the bitsets,
         * so we don't want it to be much bigger than needed.
         */
        constexpr std::size_t MAX_COMPONENTS = 128;

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

            template<class Archive>
            void serialize(Archive & archive)
            {
                archive( entity_id, deleted ); // serialize things by passing them to the archive
            }
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

            template<class Archive>
            void serialize(Archive & archive)
            {
                archive( cereal::base_class<base_component_t>(this), family_id, data ); // serialize things by passing them to the archive
            }

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
            virtual void erase_by_entity_id(ecs &ECS, const std::size_t &id)=0;
            virtual void really_delete()=0;
            virtual void save(xml_node * xml)=0;
            virtual std::size_t size()=0;

            template<class Archive>
            void serialize(Archive & archive)
            {
            }
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

            virtual void erase_by_entity_id(ecs &ECS, const std::size_t &id) override final {
                for (auto &item : components) {
                    if (item.entity_id == id) {
                        item.deleted=true;
                        impl::unset_component_mask(ECS, id, item.family_id);
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

            template<class Archive>
            void serialize(Archive & archive)
            {
                archive( cereal::base_class<base_component_store>(this), components ); // serialize things by passing them to the archive
            }

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

        /*
         * Base class for storing subscriptions to messages
         */
        struct subscription_base_t {
            virtual void deliver_messages()=0;
        };

        /* Base class for subscription mailboxes */
        struct subscription_mailbox_t {
        };

        /* Implementation class for mailbox subscriptions; stores a queue */
        template <class C>
        struct mailbox_t : subscription_mailbox_t {
            std::queue<C> messages;
        };

        /*
         * Class that holds subscriptions, and determines delivery mechanism.
         */
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

    } // End impl namespace

    /*
     * All entities are of type entity_t. They should be created with create_entity (below).
     */
    struct entity_t {

        /*
         * Default constructor - use the next available entity id #.
         */
        entity_t() {
            ++entity_t::entity_counter;
            id = entity_t::entity_counter;
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
        std::bitset<impl::MAX_COMPONENTS> component_mask;

        /*
         * Assign a component to this entity. Determines the family_id of the component type, sets the bitmask to
         * include the component, marks the component as belonging to the entity, and puts it in the appropriate
         * component store.
         */
        template<class C>
        inline entity_t * assign(ecs &ECS, C component) {
            if (deleted) throw std::runtime_error("Cannot assign to a deleted entity");
            impl::assign<C>(ECS, *this, component);
            return this;
        }

        template<class C>
        inline entity_t * assign(C component) {
            return assign<C>(default_ecs, component);
        }

        /*
         * Find a component of the specified type that belongs to the entity.
         */
        template <class C>
        inline C * component(ecs &ECS) noexcept {
            return impl::component<C>(ECS, *this);
        }

        template <class C>
        inline C * component() noexcept {
            return component<C>(default_ecs);
        }

        template<class Archive>
        void serialize(Archive & archive)
        {
            archive( component_mask, id, deleted ); // serialize things by passing them to the archive
        }
    };

    /*
     * Systems should inherit from this class.
     */
    struct base_system {
        virtual void configure() {}
        virtual void update(const double duration_ms)=0;
        std::string system_name = "Unnamed System";
        std::unordered_map<std::size_t, std::unique_ptr<impl::subscription_mailbox_t>> mailboxes;

        template<class MSG>
        void subscribe(ecs &ECS, std::function<void(MSG &message)> destination) {
            impl::subscribe<MSG>(ECS, *this, destination);
        }

        template<class MSG>
        void subscribe(std::function<void(MSG &message)> destination) {
            subscribe<MSG>(default_ecs, destination);
        }

        template<class MSG>
        void subscribe_mbox(ecs &ECS) {
            impl::subscribe_mbox<MSG>(ECS, *this);
        }

        template<class MSG>
        void subscribe_mbox() {
            subscribe_mbox<MSG>(default_ecs);
        }

        template<class MSG>
        std::queue<MSG> * mbox() {
            impl::message_t<MSG> handle(MSG{});
            auto finder = mailboxes.find(handle.family_id);
            if (finder != mailboxes.end()) {
                return &static_cast<impl::mailbox_t<MSG> *>(finder->second.get())->messages;
            } else {
                return nullptr;
            }
        }

        template<class MSG>
        void each_mbox(const std::function<void(const MSG&)> &func) {
            std::queue<MSG> * mailbox = mbox<MSG>();
            while (!mailbox->empty()) {
                MSG msg = mailbox->front();
                mailbox->pop();
                func(msg);
            }
        }
    };

    /* A pre-implemented simple system for systems that only handle messages. */
    template <class MSG>
    struct mailbox_system : public base_system {
        virtual void configure() override final {
            subscribe_mbox<MSG>();
        }

        virtual void update(const double duration_ms) override final {
            std::queue<MSG> * mailbox = base_system::mbox<MSG>();
            while (!mailbox->empty()) {
                MSG msg = mailbox->front();
                mailbox->pop();
                on_message(msg);
            }
        }

        virtual void on_message(const MSG &msg)=0;
    };

    /*
 * Class that holds an entity-component-system. This was moved to a class to allow for multiple instances.
 */
    class ecs {
    public:
        /*
         * entity(ID) is used to reference an entity. So you can, for example, do:
         * entity(12)->component<position_component>()->x = 3;
         */
        entity_t * entity(const std::size_t id) noexcept;

        /*
         * Creates an entity with a new ID #. Returns a pointer to the entity, to enable
         * call chaining. For example create_entity()->assign(foo)->assign(bar)
         */
        entity_t * create_entity();

        /*
         * Creates an entity with a specified ID #. You generally only do this during loading.
         */
        entity_t * create_entity(const std::size_t new_id);

        /*
         * Marks an entity (specified by ID#) as deleted.
         */
        inline void delete_entity(const std::size_t id) noexcept {
            auto e = entity(id);
            if (!e) return;

            e->deleted = true;
            for (auto &store : component_store) {
                if (store) store->erase_by_entity_id(*this, id);
            }
        }

        /*
         * Marks an entity as deleted.
         */
        inline void delete_entity(entity_t &e) noexcept {
            delete_entity(e.id);
        }

        /*
         * Deletes all entities
         */
        inline void delete_all_entities() noexcept  {
            for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
                delete_entity(it->first);
            }
        }

        /*
         * Marks an entity's component as deleted.
         */
        template<class C>
        inline void delete_component(const std::size_t entity_id, bool delete_entity_if_empty=false) noexcept {
            auto eptr = entity(entity_id);
            if (!eptr) return;
            entity_t e = *entity(entity_id);
            C empty_component;
            impl::component_t<C> temp(empty_component);
            if (!e.component_mask.test(temp.family_id)) return;
            for (impl::component_t<C> &component : static_cast<impl::component_store_t<impl::component_t<C>> *>(component_store[temp.family_id].get())->components) {
                if (component.entity_id == entity_id) {
                    component.deleted = true;
                    unset_component_mask(entity_id, temp.family_id, delete_entity_if_empty);
                }
            }
        }

        /*
         * Finds all entities that have a component of the type specified, and returns a
         * vector of pointers to the entities. It does not check for component deletion.
         */
        template<class C>
        inline std::vector<entity_t *> entities_with_component() {
            C empty_component;
            std::vector<entity_t *> result;
            impl::component_t<C> temp(empty_component);
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
            impl::component_t<C> temp(empty_component);
            for (impl::component_t<C> &component : static_cast<impl::component_store_t<impl::component_t<C>> *>(component_store[temp.family_id].get())->components) {
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
        inline void each(F callback) {
            std::array<size_t, sizeof...(Cs)> family_ids{ {impl::component_t<Cs>{}.family_id...} };
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
         * Variadic each_if. Use this to call a function for all entities having a discrete set of components. For example,
         * each<position, ai>([] (entity_t &e, position &pos, ai &brain) { ... code returns true if needs processing ... },
         * [] (entity_t &e, position &pos, ai &brain) { ... code ... });
         */
        template <typename... Cs, typename P, typename F>
        inline void each_if(P&& predicate, F callback) {
            std::array<size_t, sizeof...(Cs)> family_ids{ {impl::component_t<Cs>{}.family_id...} };
            for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
                if (!it->second.deleted) {
                    bool matches = true;
                    for (const std::size_t &compare : family_ids) {
                        if (!it->second.component_mask.test(compare)) {
                            matches = false;
                            break;
                        }
                    }
                    if (matches && predicate(it->second, *it->second.component<Cs>()...)) {
                        // Call the functor
                        callback(it->second, *it->second.component<Cs>()...);
                    }
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
                    for (std::unique_ptr<impl::base_component_store> &store : component_store) {
                        if (store) store->erase_by_entity_id(*this, it->second.id);
                    }
                    entities_to_delete.insert(it->second.id);
                }
            }

            // Actually delete entities
            for (const std::size_t &id : entities_to_delete) entity_store.erase(id);

            // Now we erase components
            for (std::unique_ptr<impl::base_component_store> &store : component_store) {
                if (store) store->really_delete();
            }
        }

        /*
         * Submits a message for delivery. It will be delivered to every system that has issued a subscribe or subscribe_mbox
         * call.
         */
        template <class MSG>
        inline void emit(MSG message) {
            impl::message_t<MSG> handle(message);
            if (pubsub_holder.size() > handle.family_id) {
                for (auto &func : static_cast<impl::subscription_holder_t<MSG> *>(pubsub_holder[handle.family_id].get())->subscriptions) {
                    if (std::get<0>(func) && std::get<1>(func)) {
                        std::get<1>(func)(message);
                    } else {
                        // It is destined for the system's mailbox queue.
                        auto finder = std::get<2>(func)->mailboxes.find(handle.family_id);
                        if (finder != std::get<2>(func)->mailboxes.end()) {
                            static_cast<impl::mailbox_t<MSG> *>(finder->second.get())->messages.push(message);
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
        inline void emit_deferred(MSG message) {
            impl::message_t<MSG> handle(message);
            if (pubsub_holder.size() > handle.family_id) {

                auto * subholder = static_cast<impl::subscription_holder_t<MSG> *>(pubsub_holder[handle.family_id].get());
                std::lock_guard<std::mutex> postlock(subholder->delivery_mutex);
                subholder->delivery_queue.push(message);
            }
        }

        /* Add a system to the mix */
        template<typename S, typename ...Args>
        inline void add_system( Args && ... args ) {
            system_store.push_back(std::make_unique<S>( std::forward<Args>(args) ... ));
            system_profiling.push_back(system_profiling_t{});
        }

        void delete_all_systems();

        void ecs_configure();

        void ecs_tick(const double duration_ms);

        void ecs_save(std::unique_ptr<std::ofstream> &lbfile);

        void ecs_load(std::unique_ptr<std::ifstream> &lbfile);

        std::string ecs_profile_dump();

        // The ECS component store
        std::vector<std::unique_ptr<impl::base_component_store>> component_store;

        // The ECS entity store
        std::unordered_map<std::size_t, entity_t> entity_store;

        // Mailbox system
        std::vector<std::unique_ptr<impl::subscription_base_t>> pubsub_holder;

        // Storage of systems
        std::vector<std::unique_ptr<base_system>> system_store;

        // Profile data storage
        std::vector<system_profiling_t> system_profiling;

        // Helpers
        inline void unset_component_mask(const std::size_t id, const std::size_t family_id, bool delete_if_empty) {
            auto finder = entity_store.find(id);
            if (finder != entity_store.end()) {
                finder->second.component_mask.reset(family_id);
                if (delete_if_empty && finder->second.component_mask.none()) finder->second.deleted = true;
            }
        }

        /* Delivers the queue; called at the end of each system call */
        inline void deliver_messages() {
            for (auto &holder : pubsub_holder) {
                if (holder) holder->deliver_messages();
            }
        }

        /*
         * Cereal support for save/load
         */
        template<class Archive>
        void serialize(Archive & archive)
        {
            archive( entity_store, component_store, entity_t::entity_counter, impl::base_component_t::type_counter ); // serialize things by passing them to the archive
        }
    };

    namespace impl {
        template <class C>
        inline void assign(ecs &ECS, entity_t &E, C component) {
            impl::component_t<C> temp(component);
            temp.entity_id = E.id;
            if (ECS.component_store.size() < temp.family_id+1) {
                ECS.component_store.resize(temp.family_id+1);
            }
            if (!ECS.component_store[temp.family_id]) ECS.component_store[temp.family_id] = std::move(std::make_unique<impl::component_store_t<impl::component_t<C>>>());

            static_cast<impl::component_store_t<impl::component_t<C>> *>(ECS.component_store[temp.family_id].get())->components.push_back(temp);
            E.component_mask.set(temp.family_id);
        }

        template <class C>
        inline C * component(ecs &ECS, entity_t &E) noexcept {
            C * result = nullptr;
            if (E.deleted) return result;

            C empty_component;
            impl::component_t<C> temp(empty_component);
            if (!E.component_mask.test(temp.family_id)) return result;
            for (impl::component_t<C> &component : static_cast<impl::component_store_t<impl::component_t<C>> *>(ECS.component_store[temp.family_id].get())->components) {
                if (component.entity_id == E.id) {
                    result = &component.data;
                    return result;
                }
            }
            return result;
        }

        template<class MSG>
        inline void subscribe(ecs &ECS, base_system &B, std::function<void(MSG &message)> destination) {
            MSG empty_message{};
            impl::message_t<MSG> handle(empty_message);
            if (ECS.pubsub_holder.size() < handle.family_id + 1) {
                ECS.pubsub_holder.resize(handle.family_id + 1);
            }
            if (!ECS.pubsub_holder[handle.family_id]) {
                ECS.pubsub_holder[handle.family_id] = std::move(std::make_unique<subscription_holder_t<MSG>>());
            }
            static_cast<subscription_holder_t<MSG> *>(ECS.pubsub_holder[handle.family_id].get())->subscriptions.push_back(std::make_tuple(true,destination,nullptr));
        }

        template<class MSG>
        inline void subscribe_mbox(ecs &ECS, base_system &B) {
            MSG empty_message{};
            impl::message_t<MSG> handle(empty_message);
            if (ECS.pubsub_holder.size() < handle.family_id + 1) {
                ECS.pubsub_holder.resize(handle.family_id + 1);
            }
            if (!ECS.pubsub_holder[handle.family_id]) {
                ECS.pubsub_holder[handle.family_id] = std::move(std::make_unique<subscription_holder_t<MSG>>());
            }
            std::function<void(MSG &message)> destination; // Deliberately empty
            static_cast<impl::subscription_holder_t<MSG> *>(ECS.pubsub_holder[handle.family_id].get())->subscriptions.push_back(std::make_tuple(false,destination,&B));
            B.mailboxes[handle.family_id] = std::make_unique<impl::mailbox_t<MSG>>();
        }

        inline void unset_component_mask(ecs &ECS, const std::size_t id, const std::size_t family_id, bool delete_if_empty) {
            ECS.unset_component_mask(id, family_id, delete_if_empty);
        }
    }

} // End RLTK namespace

CEREAL_REGISTER_ARCHIVE(rltk::ecs)
