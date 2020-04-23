#ifndef DYNAMIC_SUITOR_HPP_
#define DYNAMIC_SUITOR_HPP_

#include <memory>

#include <networkit/matching/SuitorMatcher.hpp>

namespace NetworKit {

class DynamicSuitorMatcher final : public SuitorMatcher {

public:
    DynamicSuitorMatcher(const Graph &G) : SuitorMatcher(G) {}
};
} // namespace NetworKit
#endif
