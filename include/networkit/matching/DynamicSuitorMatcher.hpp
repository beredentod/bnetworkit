/*
 *  SuitorMatcher.h
 *
 *  Created on: 02.12.2019
 *  Author: Michal Boron
 */

#ifndef DYNAMICSUITORMATCHER_H_
#define DYNAMICSUITORMATCHER_H_

#include <set>
#include <algorithm>
#include <vector>

#include <networkit/matching/Matcher.hpp>
#include <networkit/dynamics/GraphEvent.hpp>

namespace NetworKit {

/**
 * @ingroup matching
 * Suitor matching finding algorithm
 */
class DynSuitorMatcher: public Matcher {
public:
	DynSuitorMatcher(const Graph& G, const std::vector<node> &suitor1 = std::vector<node>(),
		const std::vector<edgeweight> &ws1 = std::vector<edgeweight>() );

	void findSuitor(node u);
	void matchSuitor(node v);

	void updateBatch(std::vector<GraphEvent> batch);
	std::set<node> event(GraphEvent e);

	count nodesNumber();
	count edgesNumber();
	std::vector<node> retrieveSuitors();
	std::vector<edgeweight> retrieveWeights();

	virtual void run();

private:

	static std::vector<node> DEFAULT_VECTOR_SUITOR;
	static std::vector<edgeweight> DEFAULT_VECTOR_WS;

	int64_t z = G->upperNodeIdBound();
	std::vector<node> suitor;	//potential suitors
	std::vector<edgeweight> ws; //weights
	std::set<node> todo; //full todo set

};

} /* namespace NetworKit */
#endif /* DYNSUITORMATCHER_H_ */