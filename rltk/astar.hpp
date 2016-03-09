/*
 A* Algorithm Implementation using STL is
 Copyright (C)2001-2005 Justin Heyes-Jones

 Permission is given by the author to freely redistribute and
 include this code in any program as long as this credit is
 given where due.
 
 COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
 INCLUDING, WITHOUT LIMITATION, WARRANTIES THAT THE COVERED CODE
 IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
 OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND
 PERFORMANCE OF THE COVERED CODE IS WITH YOU. SHOULD ANY COVERED
 CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT THE INITIAL
 DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY
 NECESSARY SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF
 WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS LICENSE. NO USE
 OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
 THIS DISCLAIMER.
 
 Use at your own risk!

 */

#pragma once

// used for text debugging
#include <iostream>
#include <stdio.h>
#include <assert.h>

// stl includes
#include <algorithm>
#include <set>
#include <vector>
#include <cfloat>

// disable warning that debugging information has lines that are truncated
// occurs in stl headers
#if defined(WIN32) && defined(_WINDOWS)
#pragma warning( disable : 4786 )
#endif

namespace rltk {
namespace astar {

template<class T> class a_star_state;

// The AStar search class. user_state is the users state space type
template<class user_state> class a_star_search
{

public:
	// data

	enum
	{
		SEARCH_STATE_NOT_INITIALISED,
		SEARCH_STATE_SEARCHING,
		SEARCH_STATE_SUCCEEDED,
		SEARCH_STATE_FAILED,
		SEARCH_STATE_OUT_OF_MEMORY,
		SEARCH_STATE_INVALID
	};

	// A node represents a possible state in the search
	// The user provided state type is included inside this type

public:

	class node_t
	{
	public:

		node_t *parent; // used during the search to record the parent of successor nodes
		node_t *child; // used after the search for the application to view the search in reverse

		float g; // cost of this node + it's predecessors
		float h; // heuristic estimate of distance to goal
		float f; // sum of cumulative cost of predecessors and self and heuristic

		node_t() : parent(0), child(0), g(0.0f), h(0.0f), f(0.0f) {}

		user_state m_user_state;
	};

	// For sorting the heap the STL needs compare function that lets us compare
	// the f value of two nodes

	class heap_compare_f
	{
	public:

		bool operator()(const node_t *x, const node_t *y) const
		{
			return x->f > y->f;
		}
	};

public:
	// methods

	// constructor just initialises private data
	a_star_search() : m_State(SEARCH_STATE_NOT_INITIALISED), m_CurrentSolutionnode_t( NULL), m_Allocatenode_tCount(0), m_CancelRequest(false)
	{
	}

	a_star_search(int Maxnode_ts) :	m_State(SEARCH_STATE_NOT_INITIALISED), m_CurrentSolutionnode_t( NULL),	m_Allocatenode_tCount(0), m_CancelRequest(false)
	{
	}

	// call at any time to cancel the search and free up all the memory
	void CancelSearch()
	{
		m_CancelRequest = true;
	}

	// Set Start and goal states
	void SetStartAndGoalStates(user_state &Start, user_state &Goal)
	{
		m_CancelRequest = false;

		m_Start = Allocatenode_t();
		m_Goal = Allocatenode_t();

		assert((m_Start != NULL && m_Goal != NULL));

		m_Start->m_user_state = Start;
		m_Goal->m_user_state = Goal;

		m_State = SEARCH_STATE_SEARCHING;

		// Initialise the AStar specific parts of the Start node_t
		// The user only needs fill out the state information

		m_Start->g = 0;
		m_Start->h = m_Start->m_user_state.GoalDistanceEstimate(m_Goal->m_user_state);
		m_Start->f = m_Start->g + m_Start->h;
		m_Start->parent = 0;

		// Push the start node on the Open list

		m_OpenList.push_back(m_Start); // heap now unsorted

		// Sort back element into heap
		push_heap(m_OpenList.begin(), m_OpenList.end(), heap_compare_f());

		// Initialise counter for search steps
		m_Steps = 0;
	}

	// Advances search one step 
	unsigned int SearchStep()
	{
		// Firstly break if the user has not initialised the search
		assert((m_State > SEARCH_STATE_NOT_INITIALISED) && (m_State < SEARCH_STATE_INVALID));

		// Next I want it to be safe to do a searchstep once the search has succeeded...
		if ((m_State == SEARCH_STATE_SUCCEEDED) || (m_State == SEARCH_STATE_FAILED))
		{
			return m_State;
		}

		// Failure is defined as emptying the open list as there is nothing left to 
		// search...
		// New: Allow user abort
		if (m_OpenList.empty() || m_CancelRequest)
		{
			FreeAllnode_ts();
			m_State = SEARCH_STATE_FAILED;
			return m_State;
		}

		// Incremement step count
		m_Steps++;

		// Pop the best node (the one with the lowest f) 
		node_t *n = m_OpenList.front(); // get pointer to the node
		pop_heap(m_OpenList.begin(), m_OpenList.end(), heap_compare_f());
		m_OpenList.pop_back();

		// Check for the goal, once we pop that we're done
		if (n->m_user_state.IsGoal(m_Goal->m_user_state))
		{
			// The user is going to use the Goal node_t he passed in 
			// so copy the parent pointer of n 
			m_Goal->parent = n->parent;
			m_Goal->g = n->g;

			// A special case is that the goal was passed in as the start state
			// so handle that here
			if (false == n->m_user_state.IsSameState(m_Start->m_user_state))
			{
				Freenode_t(n);

				// set the child pointers in each node (except Goal which has no child)
				node_t *nodeChild = m_Goal;
				node_t *nodeParent = m_Goal->parent;

				do
				{
					nodeParent->child = nodeChild;

					nodeChild = nodeParent;
					nodeParent = nodeParent->parent;

				} while (nodeChild != m_Start); // Start is always the first node by definition

			}

			// delete nodes that aren't needed for the solution
			FreeUnusednode_ts();

			m_State = SEARCH_STATE_SUCCEEDED;

			return m_State;
		}
		else // not goal
		{

			// We now need to generate the successors of this node
			// The user helps us to do this, and we keep the new nodes in
			// m_Successors ...

			m_Successors.clear(); // empty vector of successor nodes to n

			// User provides this functions and uses AddSuccessor to add each successor of
			// node 'n' to m_Successors
			bool ret = n->m_user_state.GetSuccessors(this, n->parent ? &n->parent->m_user_state : NULL);

			if (!ret)
			{

				typename std::vector<node_t *>::iterator successor;

				// free the nodes that may previously have been added 
				for (successor = m_Successors.begin();
						successor != m_Successors.end(); successor++)
				{
					Freenode_t((*successor));
				}

				m_Successors.clear(); // empty vector of successor nodes to n

				// free up everything else we allocated
				FreeAllnode_ts();

				m_State = SEARCH_STATE_OUT_OF_MEMORY;
				return m_State;
			}

			// Now handle each successor to the current node ...
			for (typename std::vector<node_t *>::iterator successor = m_Successors.begin(); successor != m_Successors.end(); successor++)
			{

				// 	The g value for this successor ...
				float newg = n->g + n->m_user_state.GetCost((*successor)->m_user_state);

				// Now we need to find whether the node is on the open or closed lists
				// If it is but the node that is already on them is better (lower g)
				// then we can forget about this successor

				// First linear search of open list to find node

				typename std::vector<node_t *>::iterator openlist_result;

				for (openlist_result = m_OpenList.begin(); openlist_result != m_OpenList.end(); openlist_result++)
				{
					if ((*openlist_result)->m_user_state.IsSameState((*successor)->m_user_state))
					{
						break;
					}
				}

				if (openlist_result != m_OpenList.end())
				{

					// we found this state on open

					if ((*openlist_result)->g <= newg)
					{
						Freenode_t((*successor));

						// the one on Open is cheaper than this one
						continue;
					}
				}

				typename std::vector<node_t *>::iterator closedlist_result;

				for (closedlist_result = m_ClosedList.begin(); closedlist_result != m_ClosedList.end(); closedlist_result++)
				{
					if ((*closedlist_result)->m_user_state.IsSameState((*successor)->m_user_state))
					{
						break;
					}
				}

				if (closedlist_result != m_ClosedList.end())
				{

					// we found this state on closed

					if ((*closedlist_result)->g <= newg)
					{
						// the one on Closed is cheaper than this one
						Freenode_t((*successor));

						continue;
					}
				}

				// This node is the best node so far with this particular state
				// so lets keep it and set up its AStar specific data ...

				(*successor)->parent = n;
				(*successor)->g = newg;
				(*successor)->h = (*successor)->m_user_state.GoalDistanceEstimate(m_Goal->m_user_state);
				(*successor)->f = (*successor)->g + (*successor)->h;

				// Remove successor from closed if it was on it

				if (closedlist_result != m_ClosedList.end())
				{
					// remove it from Closed
					Freenode_t((*closedlist_result));
					m_ClosedList.erase(closedlist_result);

					// Fix thanks to ...
					// Greg Douglas <gregdouglasmail@gmail.com>
					// who noticed that this code path was incorrect
					// Here we have found a new state which is already CLOSED

				}

				// Update old version of this node
				if (openlist_result != m_OpenList.end())
				{

					Freenode_t((*openlist_result));
					m_OpenList.erase(openlist_result);

					// re-make the heap 
					// make_heap rather than sort_heap is an essential bug fix
					// thanks to Mike Ryynanen for pointing this out and then explaining
					// it in detail. sort_heap called on an invalid heap does not work
					make_heap(m_OpenList.begin(), m_OpenList.end(),	heap_compare_f());

				}

				// heap now unsorted
				m_OpenList.push_back((*successor));

				// sort back element into heap
				push_heap(m_OpenList.begin(), m_OpenList.end(),heap_compare_f());

			}

			// push n onto Closed, as we have expanded it now

			m_ClosedList.push_back(n);

		} // end else (not goal so expand)

		return m_State; // Succeeded bool is false at this point.

	}

	// User calls this to add a successor to a list of successors
	// when expanding the search frontier
	bool AddSuccessor(user_state &State)
	{
		node_t *node = Allocatenode_t();

		if (node)
		{
			node->m_user_state = State;

			m_Successors.push_back(node);

			return true;
		}

		return false;
	}

	// Free the solution nodes
	// This is done to clean up all used node_t memory when you are done with the
	// search
	void FreeSolutionnode_ts()
	{
		node_t *n = m_Start;

		if (m_Start->child)
		{
			do
			{
				node_t *del = n;
				n = n->child;
				Freenode_t(del);

				del = NULL;

			} while (n != m_Goal);

			Freenode_t(n); // Delete the goal

		}
		else
		{
			// if the start node is the solution we need to just delete the start and goal
			// nodes
			Freenode_t(m_Start);
			Freenode_t(m_Goal);
		}

	}

	// Functions for traversing the solution

	// Get start node
	user_state *GetSolutionStart()
	{
		m_CurrentSolutionnode_t = m_Start;
		if (m_Start)
		{
			return &m_Start->m_user_state;
		}
		else
		{
			return NULL;
		}
	}

	// Get next node
	user_state *GetSolutionNext()
	{
		if (m_CurrentSolutionnode_t)
		{
			if (m_CurrentSolutionnode_t->child)
			{

				node_t *child = m_CurrentSolutionnode_t->child;

				m_CurrentSolutionnode_t = m_CurrentSolutionnode_t->child;

				return &child->m_user_state;
			}
		}

		return NULL;
	}

	// Get end node
	user_state *GetSolutionEnd()
	{
		m_CurrentSolutionnode_t = m_Goal;
		if (m_Goal)
		{
			return &m_Goal->m_user_state;
		}
		else
		{
			return NULL;
		}
	}

	// Step solution iterator backwards
	user_state *GetSolutionPrev()
	{
		if (m_CurrentSolutionnode_t)
		{
			if (m_CurrentSolutionnode_t->parent)
			{

				node_t *parent = m_CurrentSolutionnode_t->parent;

				m_CurrentSolutionnode_t = m_CurrentSolutionnode_t->parent;

				return &parent->m_user_state;
			}
		}

		return NULL;
	}

	// Get final cost of solution
	// Returns FLT_MAX if goal is not defined or there is no solution
	float GetSolutionCost()
	{
		if (m_Goal && m_State == SEARCH_STATE_SUCCEEDED)
		{
			return m_Goal->g;
		}
		else
		{
			return FLT_MAX;
		}
	}

	// For educational use and debugging it is useful to be able to view
	// the open and closed list at each step, here are two functions to allow that.

	user_state *GetOpenListStart()
	{
		float f, g, h;
		return GetOpenListStart(f, g, h);
	}

	user_state *GetOpenListStart(float &f, float &g, float &h)
	{
		iterDbgOpen = m_OpenList.begin();
		if (iterDbgOpen != m_OpenList.end())
		{
			f = (*iterDbgOpen)->f;
			g = (*iterDbgOpen)->g;
			h = (*iterDbgOpen)->h;
			return &(*iterDbgOpen)->m_user_state;
		}

		return NULL;
	}

	user_state *GetOpenListNext()
	{
		float f, g, h;
		return GetOpenListNext(f, g, h);
	}

	user_state *GetOpenListNext(float &f, float &g, float &h)
	{
		iterDbgOpen++;
		if (iterDbgOpen != m_OpenList.end())
		{
			f = (*iterDbgOpen)->f;
			g = (*iterDbgOpen)->g;
			h = (*iterDbgOpen)->h;
			return &(*iterDbgOpen)->m_user_state;
		}

		return NULL;
	}

	user_state *GetClosedListStart()
	{
		float f, g, h;
		return GetClosedListStart(f, g, h);
	}

	user_state *GetClosedListStart(float &f, float &g, float &h)
	{
		iterDbgClosed = m_ClosedList.begin();
		if (iterDbgClosed != m_ClosedList.end())
		{
			f = (*iterDbgClosed)->f;
			g = (*iterDbgClosed)->g;
			h = (*iterDbgClosed)->h;

			return &(*iterDbgClosed)->m_user_state;
		}

		return NULL;
	}

	user_state *GetClosedListNext()
	{
		float f, g, h;
		return GetClosedListNext(f, g, h);
	}

	user_state *GetClosedListNext(float &f, float &g, float &h)
	{
		iterDbgClosed++;
		if (iterDbgClosed != m_ClosedList.end())
		{
			f = (*iterDbgClosed)->f;
			g = (*iterDbgClosed)->g;
			h = (*iterDbgClosed)->h;

			return &(*iterDbgClosed)->m_user_state;
		}

		return NULL;
	}

	// Get the number of steps

	int GetStepCount()
	{
		return m_Steps;
	}

	void EnsureMemoryFreed()
	{
#if USE_FSA_MEMORY
		assert(m_Allocatenode_tCount == 0);
#endif

	}

private:
	// methods

	// This is called when a search fails or is cancelled to free all used
	// memory 
	void FreeAllnode_ts()
	{
		// iterate open list and delete all nodes
		typename std::vector<node_t *>::iterator iterOpen = m_OpenList.begin();

		while (iterOpen != m_OpenList.end())
		{
			node_t *n = (*iterOpen);
			Freenode_t(n);

			iterOpen++;
		}

		m_OpenList.clear();

		// iterate closed list and delete unused nodes
		typename std::vector<node_t *>::iterator iterClosed;

		for (iterClosed = m_ClosedList.begin();
				iterClosed != m_ClosedList.end(); iterClosed++)
		{
			node_t *n = (*iterClosed);
			Freenode_t(n);
		}

		m_ClosedList.clear();

		// delete the goal

		Freenode_t(m_Goal);
	}

	// This call is made by the search class when the search ends. A lot of nodes may be
	// created that are still present when the search ends. They will be deleted by this 
	// routine once the search ends
	void FreeUnusednode_ts()
	{
		// iterate open list and delete unused nodes
		typename std::vector<node_t *>::iterator iterOpen = m_OpenList.begin();

		while (iterOpen != m_OpenList.end())
		{
			node_t *n = (*iterOpen);

			if (!n->child)
			{
				Freenode_t(n);

				n = NULL;
			}

			iterOpen++;
		}

		m_OpenList.clear();

		// iterate closed list and delete unused nodes
		typename std::vector<node_t *>::iterator iterClosed;

		for (iterClosed = m_ClosedList.begin();
				iterClosed != m_ClosedList.end(); iterClosed++)
		{
			node_t *n = (*iterClosed);

			if (!n->child)
			{
				Freenode_t(n);
				n = NULL;

			}
		}

		m_ClosedList.clear();

	}

	// node_t memory management
	node_t *Allocatenode_t()
	{

#if !USE_FSA_MEMORY
		node_t *p = new node_t;
		return p;
#else
		node_t *address = m_FixedSizeAllocator.alloc();

		if (!address)
		{
			return NULL;
		}
		m_Allocatenode_tCount++;
		node_t *p = new (address) node_t;
		return p;
#endif
	}

	void Freenode_t(node_t *node)
	{

		m_Allocatenode_tCount--;

#if !USE_FSA_MEMORY
		delete node;
#else
		node->~node_t();
		m_FixedSizeAllocator.free(node);
#endif
	}

private:
	// data

	// Heap (simple vector but used as a heap, cf. Steve Rabin's game gems article)
	std::vector<node_t *> m_OpenList;

	// Closed list is a vector.
	std::vector<node_t *> m_ClosedList;

	// Successors is a vector filled out by the user each type successors to a node
	// are generated
	std::vector<node_t *> m_Successors;

	// State
	unsigned int m_State;

	// Counts steps
	int m_Steps;

	// Start and goal state pointers
	node_t *m_Start;
	node_t *m_Goal;

	node_t *m_CurrentSolutionnode_t;

#if USE_FSA_MEMORY
	// Memory
	FixedSizeAllocator<node_t> m_FixedSizeAllocator;
#endif

	//Debug : need to keep these two iterators around
	// for the user Dbg functions
	typename std::vector<node_t *>::iterator iterDbgOpen;
	typename std::vector<node_t *>::iterator iterDbgClosed;

	// debugging : count memory allocation and free's
	int m_Allocatenode_tCount;

	bool m_CancelRequest;

};

template<class T> class a_star_state
{
public:
	virtual ~a_star_state()
	{
	}
	virtual float GoalDistanceEstimate(T &nodeGoal) = 0; // Heuristic function which computes the estimated cost to the goal node
	virtual bool IsGoal(T &nodeGoal) = 0; // Returns true if this node is the goal node
	virtual bool GetSuccessors(a_star_search<T> *astarsearch, T *parent_node) = 0; // Retrieves all successors to this node and adds them via astarsearch.addSuccessor()
	virtual float GetCost(T &successor) = 0; // Computes the cost of traveling from this node to the successor node
	virtual bool IsSameState(T &rhs) = 0; // Returns true if this node is the same as the rhs node
};

}
}