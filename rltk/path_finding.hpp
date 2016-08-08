#pragma once

/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Path finding - interface to the A-Star system
 */

#include "astar.hpp"
#include "geometry.hpp"
#include <memory>
#include <deque>
#include <stdexcept>
#include <type_traits>

namespace rltk {

// Template class used to forward to specialize the algorithm to the user's map format and
// and behaviors defined in navigator_t. This avoids the library mandating what your map
// looks like.
template<class location_t, class navigator_t>
class map_search_node {
public:
	location_t pos;

	map_search_node() {}
	map_search_node(location_t loc) : pos(loc) {}

	float GoalDistanceEstimate(map_search_node<location_t, navigator_t> &goal) {
		float result = navigator_t::get_distance_estimate(pos, goal.pos);
		//std::cout << "GoalDistanceEstimate called (" << result << ").\n";
		return result;
	}

	bool IsGoal(map_search_node<location_t, navigator_t> &node_goal) {
		bool result = navigator_t::is_goal(pos, node_goal.pos);
		//std::cout << "IsGoal called (" << result << ").\n";
		return result;
	}

	bool GetSuccessors(AStarSearch<map_search_node<location_t, navigator_t>> * a_star_search, map_search_node<location_t, navigator_t> * parent_node) {
		//std::cout << "GetSuccessors called.\n";
		std::vector<location_t> successors;

		if (parent_node != nullptr) {		
			navigator_t::get_successors(parent_node->pos, successors);			
		} else {
			throw std::runtime_error("Null parent error.");
		}
		for (location_t loc : successors) {
			map_search_node<location_t, navigator_t> tmp(loc);
			//std::cout << " --> " << loc.x << "/" << loc.y << "\n";
			a_star_search->AddSuccessor( tmp );
		}
		return true;
	}


	float GetCost(map_search_node<location_t, navigator_t> &successor) {
		float result = navigator_t::get_cost(pos, successor.pos);
		//std::cout << "GetCost called (" << result << ").\n";
		return result;
	}

	bool IsSameState(map_search_node<location_t, navigator_t> &rhs) {
		bool result = navigator_t::is_same_state(pos, rhs.pos);
		//std::cout << "IsSameState called (" << result << ").\n";
		return result;
	}
};

// Template class used to define what a navigation path looks like
template<class location_t>
struct navigation_path {
	bool success = false;
	location_t destination;
	std::deque<location_t> steps;
};

/*
 * find_path_3d implements A*, and provides an optimization that scans a 3D Bresenham line at the beginning
 * to check for a simple line-of-sight (and paths along it). 
 * 
 * We jump through a few hoops to make sure that it will work with whatever map format you choose to use,
 * hence: it requires that the navigator_t class provide:
 * - get_x, get_y_, get_z - to translate X/Y/Z into whatever name the user wishes to utilize.
 * - get_xyz - returns a location_t given X/Y/Z co-ordinates.
 */
template<class location_t, class navigator_t>
std::shared_ptr<navigation_path<location_t>> find_path_3d(const location_t start, const location_t end) 
{
	{
		std::shared_ptr<navigation_path<location_t>> result = std::shared_ptr<navigation_path<location_t>>(new navigation_path<location_t>());
		result->success = true;
		line_func3d(navigator_t::get_x(start), navigator_t::get_y(start), navigator_t::get_z(start), navigator_t::get_x(end), navigator_t::get_y(end), navigator_t::get_z(end), [result] (int X, int Y, int Z) {
			location_t step = navigator_t::get_xyz(X,Y, Z);
			if (result->success and navigator_t::is_walkable(step)) {
				result->steps.push_back(step);
			} else {
				result->success = false;
			}
		});
		if (result->success) {
			return result;
		}
	}

	AStarSearch<map_search_node<location_t, navigator_t>> a_star_search;
	map_search_node<location_t, navigator_t> a_start(start);
	map_search_node<location_t, navigator_t> a_end(end);

	a_star_search.SetStartAndGoalStates(a_start, a_end);
	unsigned int search_state;
	unsigned int search_steps = 0;

	do {
		search_state = a_star_search.SearchStep();
		++search_steps;
	} while (search_state == AStarSearch<map_search_node<navigator_t, location_t>>::SEARCH_STATE_SEARCHING);

	if (search_state == AStarSearch<map_search_node<navigator_t, location_t>>::SEARCH_STATE_SUCCEEDED) {
		std::shared_ptr<navigation_path<location_t>> result = std::shared_ptr<navigation_path<location_t>>(new navigation_path<location_t>());
		result->destination = end;
		map_search_node<location_t, navigator_t> * node = a_star_search.GetSolutionStart();
		for (;;) {
			node = a_star_search.GetSolutionNext();
			if (!node) break;
			result->steps.push_back(node->pos);
		}
		a_star_search.FreeSolutionNodes();
		a_star_search.EnsureMemoryFreed();
		result->success = true;
		return result;
	}

	std::shared_ptr<navigation_path<location_t>> result = std::make_shared<navigation_path<location_t>>();
	a_star_search.EnsureMemoryFreed();
	return result;
}

/*
 * find_path_2d implements A*, and provides an optimization that scans a 2D Bresenham line at the beginning
 * to check for a simple line-of-sight (and paths along it). 
 * 
 * We jump through a few hoops to make sure that it will work with whatever map format you choose to use,
 * hence: it requires that the navigator_t class provide:
 * - get_x, get_y  - to translate X/Y/Z into whatever name the user wishes to utilize.
 * - get_xy - returns a location_t given X/Y/Z co-ordinates.
 */
template<class location_t, class navigator_t>
std::shared_ptr<navigation_path<location_t>> find_path_2d(const location_t start, const location_t end) 
{
	{
		std::shared_ptr<navigation_path<location_t>> result = std::shared_ptr<navigation_path<location_t>>(new navigation_path<location_t>());
		result->success = true;
		line_func(navigator_t::get_x(start), navigator_t::get_y(start), navigator_t::get_x(end), navigator_t::get_y(end), [result] (int X, int Y) {
			location_t step = navigator_t::get_xy(X,Y);
			if (result->success && navigator_t::is_walkable(step)) {
				result->steps.push_back(step);
			} else {
				result->success = false;
			}
		});
		if (result->success) {
			return result;
		}
	}

	AStarSearch<map_search_node<location_t, navigator_t>> a_star_search;
	map_search_node<location_t, navigator_t> a_start(start);
	map_search_node<location_t, navigator_t> a_end(end);

	a_star_search.SetStartAndGoalStates(a_start, a_end);
	unsigned int search_state;
	unsigned int search_steps = 0;

	do {
		search_state = a_star_search.SearchStep();
		++search_steps;
	} while (search_state == AStarSearch<map_search_node<navigator_t, location_t>>::SEARCH_STATE_SEARCHING);

	if (search_state == AStarSearch<map_search_node<navigator_t, location_t>>::SEARCH_STATE_SUCCEEDED) {
		std::shared_ptr<navigation_path<location_t>> result = std::shared_ptr<navigation_path<location_t>>(new navigation_path<location_t>());
		result->destination = end;
		map_search_node<location_t, navigator_t> * node = a_star_search.GetSolutionStart();
		for (;;) {
			node = a_star_search.GetSolutionNext();
			if (!node) break;
			result->steps.push_back(node->pos);
		}
		a_star_search.FreeSolutionNodes();
		a_star_search.EnsureMemoryFreed();
		result->success = true;
		return result;
	}

	std::shared_ptr<navigation_path<location_t>> result = std::make_shared<navigation_path<location_t>>();
	a_star_search.EnsureMemoryFreed();
	return result;
}

/*
 * Implements a simple A-Star path, with no line-search optimization. This has the benefit of avoiding
 * requiring as much additional translation between the template and your preferred map format, at the
 * expense of being potentially slower for some paths.
 */
template<class location_t, class navigator_t>
std::shared_ptr<navigation_path<location_t>> find_path(const location_t start, const location_t end) 
{
	AStarSearch<map_search_node<location_t, navigator_t>> a_star_search;
	map_search_node<location_t, navigator_t> a_start(start);
	map_search_node<location_t, navigator_t> a_end(end);

	a_star_search.SetStartAndGoalStates(a_start, a_end);
	unsigned int search_state;
	std::size_t search_steps = 0;

	do {
		search_state = a_star_search.SearchStep();
		++search_steps;
	} while (search_state == AStarSearch<map_search_node<navigator_t, location_t>>::SEARCH_STATE_SEARCHING);

	if (search_state == AStarSearch<map_search_node<navigator_t, location_t>>::SEARCH_STATE_SUCCEEDED) {
		std::shared_ptr<navigation_path<location_t>> result = std::shared_ptr<navigation_path<location_t>>(new navigation_path<location_t>());
		result->destination = end;
		map_search_node<location_t, navigator_t> * node = a_star_search.GetSolutionStart();
		for (;;) {
			node = a_star_search.GetSolutionNext();
			if (!node) break;
			result->steps.push_back(node->pos);
		}
		a_star_search.FreeSolutionNodes();
		a_star_search.EnsureMemoryFreed();
		result->success = true;
		return result;
	}

	std::shared_ptr<navigation_path<location_t>> result = std::make_shared<navigation_path<location_t>>();
	a_star_search.EnsureMemoryFreed();
	return result;
}

}
