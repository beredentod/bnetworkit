/*
 *  SuitorMatcher.h
 *
 *  Created on: 27.08.2019
 *  Author: Michal Boron
 */

#ifndef SUITORMATCHER_H_
#define SUITORMATCHER_H_

#include <algorithm>
#include <vector>

#include <networkit/matching/Matcher.hpp>

namespace NetworKit {

/**
 * @ingroup matching
 * Suitor matching finding algorithm
 */
class SuitorMatcher final : public Matcher {
    std::vector<node> suitor;
    std::vector<edgeweight> ws;

    void findSuitor(node u);
    void matchSuitor(node v);

public:
    SuitorMatcher(const Graph &G);

    void run() override;

    std::vector<node> retrieveSuitors();
    std::vector<edgeweight> retrieveWeights();
    std::vector<Graph::NeighborWeightIterator> neighborIterators;
};

} /* namespace NetworKit */
#endif /* SUITORMATCHER_H_ */
