#include <cassert>

#include <networkit/matching/DynamicSuitorMatcher.hpp>

namespace NetworKit {

void DynamicSuitorMatcher::insertBatch(const std::vector<GraphEvent> &additions) {
    std::copy(ws.begin(), ws.end(), wsPrev.begin());

    auto processNode = [&](const node x, const edgeweight weight) {
        ++additionsPerNode[x];
        if (!affected[x] && weight > wsPrev[x]) {
            affected[x] = 1;
            ws[x] = 0;
            suitor[x] = none;
            affectedNodes.push_back(x);
        }
    };

    for (const auto &addition : additions) {
        assert(addition.type == GraphEvent::Type::EDGE_ADDITION);
        processNode(addition.u, addition.w);
        processNode(addition.v, addition.w);
    }
}

void DynamicSuitorMatcher::doUpdate() {
    for (const auto u : affectedNodes)
        findSuitor(u);
    for (const auto u : affectedNodes)
        matchSuitor(u);
}
} /* namespace NetworKit */
