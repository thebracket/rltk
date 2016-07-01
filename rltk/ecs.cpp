#include "ecs.hpp"

namespace rltk {

std::size_t base_component_t::type_counter = 0;
std::size_t base_message_t::type_counter = 0;
std::size_t entity_t::entity_counter = 1; // Not using zero since it is used as null so often
std::vector<std::unique_ptr<base_component_store>> component_store;
std::unordered_map<std::size_t, entity_t> entity_store;
std::vector<std::unique_ptr<base_system>> system_store;
std::vector<std::unique_ptr<subscription_base_t>> pubsub_holder;
std::vector<system_profiling_t> system_profiling;

}