/*
 *  SuitorMatcher.cpp
 *
 *  Created on: 27.08.2019
 */

#include <networkit/matching/SuitorMatcher.hpp>

namespace NetworKit {

SuitorMatcher::SuitorMatcher(const Graph &G) : Matcher(G) {
    if (G.isDirected())
        throw std::runtime_error("Matcher only defined for undirected graphs");

    // preparation of the two arrays for finding suitor
    suitor.resize(G.upperNodeIdBound(), none);
    ws.resize(G.upperNodeIdBound(), 0);
    neighborIterators.reserve(G.upperNodeIdBound());
    G.forNodes(
        [&](const node u) { neighborIterators.emplace_back(G.weightNeighborRange(u).begin()); });
}

void SuitorMatcher::findSuitorOriginal(node current) {
    bool done = false;

    do {

        node partner = suitor[current];
        edgeweight heaviest = ws[current];

        G->forNeighborsOf(current, [&](const node v, const edgeweight weight) {
            if (weight > heaviest && weight > ws[v]) {
                partner = v;
                heaviest = weight;
            }
        });

        done = true;

        if (heaviest > 0) {
            node y = suitor[partner];
            suitor[partner] = current;
            ws[partner] = heaviest;

            // if the current vertex already has a suitor
            if (y != none) {
                current = y;
                done = false;
            }
        }
    } while (!done);
}

void SuitorMatcher::findSuitor(node current) {
    bool done = false;

    do {

        node partner = suitor[current];
        edgeweight heaviest = ws[current];

        for (auto &iter = neighborIterators[current]; iter != G->weightNeighborRange(current).end();
             ++iter) {
            const auto [v, weight] = *iter;
            if (weight > heaviest && weight > ws[v]) {
                partner = v;
                heaviest = weight;
                ++iter;
                break;
            }
        }

        done = true;

        if (heaviest > 0) {
            node y = suitor[partner];
            suitor[partner] = current;
            ws[partner] = heaviest;

            // if the current vertex already has a suitor
            if (y != none) {
                current = y;
                done = false;
            }
        }
    } while (!done);
}

void SuitorMatcher::matchSuitor(node u) {
    if (suitor[u] == none)
        return;

    node v = suitor[u];
    assert(u != v);
    if (!M.isMatched(u) && !M.isMatched(v))
        M.match(u, v);
}

void SuitorMatcher::run() {
    G->forNodes([&](const auto u) { findSuitor(u); });

    // match vertices with its suitors
    G->forNodes([&](const auto u) { matchSuitor(u); });
}

void SuitorMatcher::runOriginal() {
    G->forNodes([&](const auto u) { findSuitorOriginal(u); });
    // match vertices with its suitors
    G->forNodes([&](const auto u) { matchSuitor(u); });
}

} /* namespace NetworKit */
