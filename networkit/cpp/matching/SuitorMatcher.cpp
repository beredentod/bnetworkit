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
}

void SuitorMatcher::init() {
    // preparation of the two arrays for finding suitor
    std::fill(suitor.begin(), suitor.end(), none);
    suitor.resize(G->upperNodeIdBound(), none);
    std::fill(ws.begin(), ws.end(), 0);
    ws.resize(G->upperNodeIdBound(), 0);
    neighborIterators.clear();
    neighborIterators.reserve(G->upperNodeIdBound());
    G->forNodes(
        [&](const node u) { neighborIterators.emplace_back(G->weightNeighborRange(u).begin()); });
    M.clear();
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

        if (partner != none && heaviest > ws[partner]) {
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

        if (partner != none && heaviest > ws[partner]) {
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

    if (suitor[u] != none && u == suitor[suitor[u]]) {
        M.match(u, suitor[u]);
    }
}

void SuitorMatcher::run() {
    init();
    G->forNodes([&](const auto u) { findSuitor(u); });
    // match vertices with its suitors
    G->forNodes([&](const auto u) { matchSuitor(u); });
}

void SuitorMatcher::runOriginal() {
    init();
    G->forNodes([&](const auto u) { findSuitorOriginal(u); });
    // match vertices with its suitors
    G->forNodes([&](const auto u) { matchSuitor(u); });
}

} /* namespace NetworKit */