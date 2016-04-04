#include "ecs.hpp"

namespace rltk {

std::size_t base_component_t::type_counter = 0;
std::size_t entity_t::entity_counter = 0;
std::vector<std::unique_ptr<base_component_store>> component_store;

}