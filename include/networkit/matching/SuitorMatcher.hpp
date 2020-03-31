/*
 *  SuitorMatcher.h
 *
 *  Created on: 27.08.2019
 *  Author: Michal Boron
 */

#ifndef SUITORMATCHER_H_
#define SUITORMATCHER_H_

#include <set>
#include <algorithm>
#include <vector>

#include <networkit/matching/Matcher.hpp>

namespace NetworKit {

/**
 * @ingroup matching
 * Suitor matching finding algorithm
 */
class SuitorMatcher: public Matcher {
public:
	SuitorMatcher(const Graph& G);

	void findSuitor(node u);
	void matchSuitor(node v);

	virtual void run();

	count nodesNumber();
	count edgesNumber();
	std::vector<node> retrieveSuitors();
	std::vector<edgeweight> retrieveWeights();


private:
	int64_t z = G->upperNodeIdBound();
	std::vector<node> suitor;
	std::vector<edgeweight> ws;
	std::vector< std::multimap <edgeweight, node, std::greater<edgeweight> > > neighbors;

};

} /* namespace NetworKit */
#endif /* SUITORMATCHER_H_ */
