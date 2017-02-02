#include "ecs.hpp"
#include "../cereal/types/polymorphic.hpp"
#include "../cereal/archives/xml.hpp"

namespace rltk {

std::size_t impl::base_component_t::type_counter = 1;
std::size_t base_message_t::type_counter = 1;
std::size_t entity_t::entity_counter{1}; // Not using zero since it is used as null so often
ecs default_ecs;

entity_t * ecs::entity(const std::size_t id) noexcept {
	entity_t * result = nullptr;
	auto finder = entity_store.find(id);
	if (finder == entity_store.end()) return result;
	if (finder->second.deleted) return result;
	result = &finder->second;
	return result;
}

entity_t * ecs::create_entity() {
    entity_t new_entity;
    while (entity_store.find(new_entity.id) != entity_store.end()) {
        ++entity_t::entity_counter;
        new_entity.id = entity_t::entity_counter;
    }
    //std::cout << "New Entity ID#: " << new_entity.id << "\n";

    entity_store.emplace(new_entity.id, new_entity);
    return entity(new_entity.id);
}

entity_t * ecs::create_entity(const std::size_t new_id) {
	entity_t new_entity(new_id);
    if (entity_store.find(new_entity.id) != entity_store.end()) {
        throw std::runtime_error("WARNING: Duplicate entity ID. Odd things will happen\n");
    }
	entity_store.emplace(new_entity.id, new_entity);
	return entity(new_entity.id);
}

void ecs::each(std::function<void(entity_t &)> &&func) {
	for (auto it=entity_store.begin(); it!=entity_store.end(); ++it) {
		if (!it->second.deleted) {
			func(it->second);
		}
	}
}


void ecs::delete_all_systems() {
	system_store.clear();
	system_profiling.clear();
	pubsub_holder.clear();
}

void ecs::ecs_configure() {
	for (std::unique_ptr<base_system> & sys : system_store) {
		sys->configure();
	}
}

void ecs::ecs_tick(const double duration_ms) {
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

void ecs::ecs_save(std::unique_ptr<std::ofstream> &lbfile) {
    cereal::XMLOutputArchive oarchive(*lbfile);
    oarchive(*this);
}

void ecs::ecs_load(std::unique_ptr<std::ifstream> &lbfile) {
	entity_store.clear();
	component_store.clear();
    cereal::XMLInputArchive iarchive(*lbfile);
    iarchive(*this);
    std::cout << "Loaded " << entity_store.size() << " entities, and " << component_store.size() << " component types.\n";
}

std::string ecs::ecs_profile_dump() {
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
