 /*
 *  SuitorMatcher.cpp
 *
 *  Created on: 27.08.2019
 */

//#include "SuitorMatcher.hpp"
#include<networkit/matching/SuitorMatcher.hpp>

namespace NetworKit {

SuitorMatcher::SuitorMatcher(const Graph& G): Matcher(G)
{
	if (G.isDirected()) throw std::runtime_error("Matcher only defined for undirected graphs");

	//preparation of the two arrays for finding suitor
	for (int i=0;i<z;i++)
			suitor.push_back(none);

	for (int i=0;i<z;i++)
		ws.push_back(0);

}

count SuitorMatcher::nodesNumber()
{
	return G->numberOfNodes();
}

count SuitorMatcher::edgesNumber()
{
	return G->numberOfEdges();
}

std::vector<node> SuitorMatcher::retrieveSuitors()
{
	return suitor;
}

std::vector<edgeweight> SuitorMatcher::retrieveWeights()
{
	return ws;
}

void SuitorMatcher::findSuitor(node v)
{
	bool done = false;

	node n1 = none, n2 = none; //to avoid falling into an infinite loop

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

void SuitorMatcher::matchSuitor(node v)
{
	if (suitor[v] == none)
		return;

	node s = v;
	node t = suitor[s];
	if (!M.isMatched(s) && !M.isMatched(t) && s != t)
	{
		M.match(s,t);
	}
}

void SuitorMatcher::run() 
{
	for (auto u: G->nodeRange())
		findSuitor(u);

	//match vertices with its suitors

	for (auto v: G->nodeRange())
		matchSuitor(v);

}


} /* namespace NetworKit */
