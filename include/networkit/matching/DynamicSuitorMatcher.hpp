#ifndef DYNAMIC_SUITOR_HPP_
#define DYNAMIC_SUITOR_HPP_

#include <networkit/auxiliary/Log.hpp>
#include <networkit/dynamics/GraphEvent.hpp>
#include <networkit/matching/SuitorMatcher.hpp>

namespace NetworKit {

class DynamicSuitorMatcher final : public SuitorMatcher {

    void setAffected(node u);

public:
    DynamicSuitorMatcher(const Graph &G) : SuitorMatcher(G) {
        affected.resize(G.upperNodeIdBound(), 0);
        wsPrev.resize(G.upperNodeIdBound());
        affectedNodes.reserve(G.numberOfNodes());
    }

    void findAffectedAfterEdgeAdditions(const std::vector<GraphEvent> &additions);
    void findAffectedAfterEdgeRemovals(const std::vector<GraphEvent> &removals);
    void doUpdate();

    std::vector<unsigned char> affected;
    std::vector<edgeweight> wsPrev;
    std::vector<node> affectedNodes;
};
} // namespace NetworKit
#endif
