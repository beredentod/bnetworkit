#include <cassert>

#include <networkit/matching/DynamicSuitorMatcher.hpp>

namespace NetworKit {

void DynamicSuitorMatcher::setAffected(node u) {
    assert(!affected[u]);
    affected[u] = 1;
    ws[u] = 0;
    suitor[u] = none;
    affectedNodes.push_back(u);
}

void DynamicSuitorMatcher::findAffectedAfterEdgeAdditions(const std::vector<GraphEvent> &additions) {
    std::copy(ws.begin(), ws.end(), wsPrev.begin());

    auto processNode = [&](const node u, const node v, const edgeweight weight) {
        if (!affected[u] && (weight > wsPrev[u] || (weight == wsPrev[u] && v > M.mate(u))))
            setAffected(u);
    };

    for (const auto &addition : additions) {
        assert(addition.type == GraphEvent::Type::EDGE_ADDITION);
        processNode(addition.u, addition.v, addition.w);
        processNode(addition.v, addition.u, addition.w);
    }
}

void DynamicSuitorMatcher::findAffectedAfterEdgeRemovals(const std::vector<GraphEvent> &removals) {
    std::copy(ws.begin(), ws.end(), wsPrev.begin());

    // Called at most once per node
    auto processNode = [&](const node u, const edgeweight weight) {
        if (!affected[u])
            setAffected(u);
        G->forNeighborsOf(u, [&](const node x) {
            if (affected[x])
                return;
            if (!M.isMatched(x))
                setAffected(x);
            else if (wsPrev[x] < weight || (wsPrev[x] == weight && M.mate(x) < u)) {
                setAffected(x);
                if (!affected[M.mate(x)])
                    setAffected(M.mate(x));
            }
        });
    };

    for (const auto &removal : removals) {
        assert(removal.type == GraphEvent::Type::EDGE_REMOVAL);
        // If edge not in the matching, ignore removal
        if (M.mate(removal.u) != removal.v)
            continue;
        const auto weight = wsPrev[removal.u];
        assert(weight == wsPrev[removal.v]);
        processNode(removal.u, weight);
        processNode(removal.v, weight);
    }
}

void DynamicSuitorMatcher::doUpdate() {
#ifndef NDEBUG
    checkSortedEdges();
#endif
    for (const auto u : affectedNodes)
        findSuitor(u);
    for (const auto u : affectedNodes)
        matchSuitor(u);
#ifndef NDEBUG
    checkMatching();
#endif
}

} /* namespace NetworKit */