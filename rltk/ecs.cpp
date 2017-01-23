#include "ecs.hpp"

namespace rltk {

std::size_t base_component_t::type_counter = 1;
std::size_t base_message_t::type_counter = 1;
std::atomic<std::size_t> entity_t::entity_counter{1}; // Not using zero since it is used as null so often
std::vector<std::unique_ptr<base_component_store>> component_store;
std::unordered_map<std::size_t, entity_t> entity_store;
std::vector<std::unique_ptr<base_system>> system_store;
std::vector<std::unique_ptr<subscription_base_t>> pubsub_holder;
std::vector<system_profiling_t> system_profiling;

void unset_component_mask(const std::size_t id, const std::size_t family_id, bool delete_if_empty) {
	auto finder = entity_store.find(id);
	if (finder != entity_store.end()) {
		finder->second.component_mask.reset(family_id);
		if (delete_if_empty && finder->second.component_mask.none()) finder->second.deleted = true;
	}
}

entity_t * entity(const std::size_t id) noexcept {
	entity_t * result = nullptr;
	auto finder = entity_store.find(id);
	if (finder == entity_store.end()) return result;
	if (finder->second.deleted) return result;
	result = &finder->second;
	return result;
}

entity_t * create_entity() {
    entity_t new_entity;
    while (entity_store.find(new_entity.id) != entity_store.end()) {
        ++entity_t::entity_counter;
        new_entity.id = entity_t::entity_counter;
    }
    //std::cout << "New Entity ID#: " << new_entity.id << "\n";

    entity_store.emplace(new_entity.id, new_entity);
    return entity(new_entity.id);
}

entity_t * create_entity(const std::size_t new_id) {
	entity_t new_entity(new_id);
    if (entity_store.find(new_entity.id) != entity_store.end()) {
        throw std::runtime_error("WARNING: Duplicate entity ID. Odd things will happen\n");
    }
	entity_store.emplace(new_entity.id, new_entity);
	return entity(new_entity.id);
}

void each(std::function<void(entity_t &)> &&func) {
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (!it->second.deleted) {
			func(it->second);
		}
	}
}

void delete_entity(const std::size_t id) noexcept {
	auto e = entity(id);
	if (!e) return;

	e->deleted = true;
	for (auto &store : component_store) {
		if (store) store->erase_by_entity_id(id);
	}
}

void delete_entity(entity_t &e) noexcept {
	delete_entity(e.id);
}

void delete_all_entities() noexcept {
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		delete_entity(it->first);
	}
}

void ecs_garbage_collect() {
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

void delete_all_systems() {
	system_store.clear();
	system_profiling.clear();
	pubsub_holder.clear();
}

void ecs_configure() {
	for (std::unique_ptr<base_system> & sys : system_store) {
		sys->configure();
	}
}

void ecs_tick(const double duration_ms) {
	std::size_t count = 0;
	for (std::unique_ptr<base_system> & sys : system_store) {
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
		sys->update(duration_ms);
		deliver_messages();
		std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
		double duration = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count());

		system_profiling[count].last = duration;
		if (duration > system_profiling[count].worst) system_profiling[count].worst = duration;
		if (duration < system_profiling[count].best) system_profiling[count].best = duration;
		++count;
	}
	ecs_garbage_collect();
}

void ecs_save(std::unique_ptr<std::ofstream> lbfile) {
	// Initialize the XML system
	xml_writer writer(std::move(lbfile), "ECS");

	// Store the number of entities and their ID numbers
	xml_node * entities = writer.add_node("entities");
	entities->add_value( "entityCount", std::to_string(entity_store.size()) );
    for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		entities->add_value("entityId", std::to_string(it->first));
	}

	// Store the last entity number
	entities->add_value("lastEntity", std::to_string(entity_t::entity_counter));

	// For each component type
	xml_node * components = writer.add_node("components");

	std::size_t number_of_components = 0;
	for (auto &it : component_store) {
		if (it) number_of_components += it->size();
	}
	components->add_value("componentCount", std::to_string(number_of_components));

    for (auto &it : component_store) {
        if (it) it->save(components);
    }

	writer.commit();
}

void ecs_load(std::unique_ptr<std::ifstream> lbfile, const std::function<void(xml_node *, std::size_t, std::string)> &helper) {
	entity_store.clear();
	component_store.clear();
    xml_reader reader(std::move(lbfile));

    xml_node * entity_list = reader.get()->find("entities");
    //std::size_t number_of_entities = entity_list->val<std::size_t>("entityCount");
	std::size_t last_entity = entity_list->val<std::size_t>("lastEntity");
    for (const auto &e : entity_list->values) {
        if (e.first == "entityId") {
            std::size_t entity_id;
            std::stringstream ss;
            ss << e.second;
            ss >> entity_id;
            create_entity(entity_id);
        }
    }
	entity_t::entity_counter = last_entity;

    xml_node * component_list = reader.get()->find("components");
    //std::size_t number_of_components = component_list->val<std::size_t>("componentCount");
    for (xml_node &comp : component_list->children) {
        std::size_t entity_id = comp.val<std::size_t>("entity_id");
        helper(&comp, entity_id, comp.name);
    }
}

std::string ecs_profile_dump() {
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
