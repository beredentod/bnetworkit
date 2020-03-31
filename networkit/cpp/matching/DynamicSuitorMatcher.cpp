 /*
 *  SuitorMatcher.cpp
 *
 *  Created on: 02.12.2019
 */

//#include "DynamicSuitorMatcher.hpp"
#include<networkit/matching/DynamicSuitorMatcher.hpp>

namespace NetworKit {

DynSuitorMatcher::DynSuitorMatcher(const Graph& G, const std::vector<node> &suitor1,
	const std::vector<edgeweight> &ws1): Matcher(G)
{
	if (G.isDirected()) throw std::runtime_error("Matcher only defined for undirected graphs");

	suitor = suitor1; //combine the retrieved suitor with local
	ws = ws1; //combine the retrieved ws with local

}

count DynSuitorMatcher::nodesNumber()
{
	return G->numberOfNodes();
}

count DynSuitorMatcher::edgesNumber()
{
	return G->numberOfEdges();
}

std::vector<node> DynSuitorMatcher::retrieveSuitors()
{
	return suitor;
}

std::vector<edgeweight> DynSuitorMatcher::retrieveWeights()
{
	return ws;
}

void DynSuitorMatcher::findSuitor(node v)
{
	bool done = false;

	node n1 = none, n2 = none;

	while (!done)
	{
		if (n2 == v)
		{
			return;
		}

		node partner = suitor[v];
		edgeweight heaviest = ws[v];

		//look for the heaviest edges adjacent to the current vertex
		for ( node e: G->neighborRange(v) )
		{
			edgeweight curr_edge = G->weight(v, e);
			if ( curr_edge > heaviest && curr_edge > ws[e] )
			{
				partner = e;
				heaviest = curr_edge;
			}
		}

		done = true;

		if (heaviest > 0)
		{
			int y = suitor[partner];
			suitor[partner] = v;
			ws[partner] = heaviest;

			//if the current vertex already has a suitor
			if (y != none)
			{
				n2 = n1;
				n1 = v;
				v = y;
				done = false;
			}
			
		}
	}
}

void DynSuitorMatcher::matchSuitor(node v)
{
	if (suitor[v] == none)
		return;

	node s = v;
	node t = suitor[s];
	if (!M.isMatched(s) && !M.isMatched(t) && s != t)
	{
		M.match(s,t);

		//std::cout<<"[INFO ]: Match: ("<<s<<", "<<t<<") -- "<<GC.weight(s,t)<<"\n";
	}
}

void DynSuitorMatcher::updateBatch(std::vector<GraphEvent> batch)
{
	printf("[INFO ]: Updating Batch\n");
	for (auto e: batch)
	{
		std::set<node> stage_todo = event(e);
		for (auto v: stage_todo)
			todo.insert(v);
	}
}

void DynSuitorMatcher::run()
{
	for (node v: todo)
	{
		if (suitor[v] == none)
			findSuitor(v);
	}

	for (node v: todo)
	{
		matchSuitor(v);
	}
}


std::set<node> DynSuitorMatcher::event(GraphEvent e)
{
	std::set<node> todo;

	if (e.type == GraphEvent::EDGE_ADDITION)
	{
		node u = e.u, v = e.v;
		edgeweight w = e.w;

		node um = none, vm = none; //mates of u and v
		if(M.isMatched(u))
			um = M.mate(u);
		if(M.isMatched(v))
			vm = M.mate(v);	

		if(um != none && vm != none)
		{
			edgeweight eW1 = G->weight(um, u);
			edgeweight ew2 = G->weight(vm, v);

			if (eW1 + ew2 < w)
			{
				M.unmatch(u, um);
				suitor[um] = none;
				suitor[u] = none;
				ws[um] = 0;
				ws[u] = 0;
				todo.insert(um);
				todo.insert(u);	
				M.unmatch(v, vm);
				suitor[vm] = none;
				suitor[v] = none;
				ws[vm] = 0;
				ws[v] = 0;
				todo.insert(vm);
				todo.insert(v);			
			}
		}
		else if(um != none)
		{
			edgeweight eW = G->weight(um, u);
			if (eW < w)
			{
				M.unmatch(u, um);
				suitor[um] = none;
				suitor[u] = none;
				ws[um] = 0;
				ws[u] = 0;
				todo.insert(um);
				todo.insert(u);				
			}
		}
		else if(vm != none)
		{
			edgeweight eW = G->weight(vm, v);
			if (eW < w)
			{
				M.unmatch(v, vm);
				suitor[vm] = none;
				suitor[v] = none;
				ws[vm] = 0;
				ws[v] = 0;
				todo.insert(vm);
				todo.insert(v);
			}
		}

		return todo;
	}

	if (e.type == GraphEvent::EDGE_REMOVAL)
	{
		node u = e.u, v = e.v;

		node um = none, vm = none; //mates of u and v
		if(M.isMatched(u))
			um = M.mate(u);
		if(M.isMatched(v))
			vm = M.mate(v);

		/*if(M.isMatched(v) && M.isMatched(u) && um != v || !M.isMatched(v) || M.isMatched(u))
			return todo;*/

		if (um == v && vm == u)
		{
			for ( node e: G->neighborRange(v) )
			{
				suitor[e] = none;
				ws[e] = 0;
				todo.insert(e);
				if(M.isMatched(e))
					M.unmatch((e), M.mate(e));
			}
			for ( node e: G->neighborRange(u) )
			{
				suitor[e] = none;
				ws[e] = 0;
				todo.insert(e);
				if(M.isMatched(e))
					M.unmatch((e), M.mate(e));
			}
		}

		if(um != none)
		{
			M.unmatch(u, um);
			suitor[um] = none;
			ws[um] = 0;
			todo.insert(um);
		}
		if(vm != none)
		{
			M.unmatch(v, vm);
			suitor[vm] = none;
			ws[vm] = 0;
			todo.insert(vm);
		}

		todo.insert(u);
		todo.insert(v);
		suitor[v] = none;
		suitor[u] = none;
		ws[u] = 0;
		ws[v] = 0;

		return todo;

	}

	if (e.type == GraphEvent::EDGE_WEIGHT_UPDATE)
	{
		node u = e.u, v = e.v;
		edgeweight w = e.w;

		node um = none, vm = none; //mates of u and v
		if(M.isMatched(u))
			um = M.mate(u);
		if(M.isMatched(v))
			vm = M.mate(v);	

		if(um != none && vm != none)
		{
			edgeweight eW1 = G->weight(um, u);
			edgeweight ew2 = G->weight(vm, v);

			if (eW1 + ew2 < w)
			{
				M.unmatch(u, um);
				suitor[um] = none;
				suitor[u] = none;
				ws[um] = 0;
				ws[u] = 0;
				todo.insert(um);
				todo.insert(u);	
				M.unmatch(v, vm);
				suitor[vm] = none;
				suitor[v] = none;
				ws[vm] = 0;
				ws[v] = 0;
				todo.insert(vm);
				todo.insert(v);			
			}
		}
		else if(um != none)
		{
			edgeweight eW = G->weight(um, u);
			if (eW < w)
			{
				M.unmatch(u, um);
				suitor[um] = none;
				suitor[u] = none;
				ws[um] = 0;
				ws[u] = 0;
				todo.insert(um);
				todo.insert(u);				
			}
		}
		else if(vm != none)
		{
			edgeweight eW = G->weight(vm, v);
			if (eW < w)
			{
				M.unmatch(v, vm);
				suitor[vm] = none;
				suitor[v] = none;
				ws[vm] = 0;
				ws[v] = 0;
				todo.insert(vm);
				todo.insert(v);
			}
		}

		return todo;
	}

	if (e.type == GraphEvent::EDGE_WEIGHT_INCREMENT)
	{
		node u = e.u, v = e.v;

		node um = none, vm = none; //mates of u and v
		if(M.isMatched(u))
			um = M.mate(u);
		if(M.isMatched(v))
			vm = M.mate(v);

		int w_uv = G->weight(u, v);
		
		if(um != none)
		{
			if (um == v)
				return todo;

			int w_u = G->weight(u, um);
			if (w_u >= w_uv)
				return todo;

			M.unmatch(u, um);
			suitor[um] = none;
			ws[um] = 0;
			todo.insert(um);
		}
		if(vm != none)
		{
			int w_v = G->weight(v, vm);
			if (w_v >= w_uv)
				return todo;
			
			M.unmatch(v, vm);
			suitor[vm] = none;
			ws[vm] = 0;
			todo.insert(vm);
		}

		todo.insert(u);
		todo.insert(v);
		suitor[v] = none;
		suitor[u] = none;
		ws[u] = 0;
		ws[v] = 0;

		return todo;		
	}

}

} /* namespace NetworKit */
