#ifndef DYNAMIC_SUITOR_HPP_
#define DYNAMIC_SUITOR_HPP_

#include <networkit/dynamics/GraphEvent.hpp>
#include <networkit/matching/SuitorMatcher.hpp>

namespace NetworKit {

class DynamicSuitorMatcher final : public SuitorMatcher {

public:
    DynamicSuitorMatcher(const Graph &G) : SuitorMatcher(G) {}

    void insertBatch(const std::vector<GraphEvent> &insertions);
};
} // namespace NetworKit
#endif
