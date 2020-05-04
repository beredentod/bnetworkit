/*
 * MatcherGTest.cpp
 *
 *  Created on: Feb 7, 2013
 *      Author: Henning
 */

#include <gtest/gtest.h>

#include <networkit/auxiliary/Random.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/graph/GraphTools.hpp>
#include <networkit/io/DibapGraphReader.hpp>
#include <networkit/io/METISGraphReader.hpp>
#include <networkit/io/NetworkitBinaryReader.hpp>
#include <networkit/matching/DynamicSuitorMatcher.hpp>
#include <networkit/matching/LocalMaxMatcher.hpp>
#include <networkit/matching/Matcher.hpp>
#include <networkit/matching/Matching.hpp>
#include <networkit/matching/PathGrowingMatcher.hpp>
#include <networkit/matching/SuitorMatcher.hpp>

namespace NetworKit {

class MatcherGTest : public testing::Test {};

TEST_F(MatcherGTest, testSuitorMatcher) {
    Graph G(4, true, false);
    G.addEdge(0, 1, 6);
    G.addEdge(0, 2, 5);
    G.addEdge(1, 2, 8);
    G.addEdge(1, 3, 3);
    G.addEdge(2, 3, 11);
    G = NetworkitBinaryReader{}.read("/work/global/angriman/graphs/advogato.nkb");
    if (!G.isWeighted())
        G = GraphTools::toWeighted(G);
    if (G.isDirected())
        G = GraphTools::toUndirected(G);
    G.removeSelfLoops();
    G.removeMultiEdges();
    Aux::Random::setSeed(1, false);
    G.forEdges([&](node u, node v) { G.setWeight(u, v, Aux::Random::probability()); });
    G.sortOutEdgesByWeight(std::greater<edgeweight>());
    SuitorMatcher sm(G);
    sm.run();
    const auto m = sm.getMatching();
    INFO("Weight = ", m.weight(G));
    SuitorMatcher sm2(G);
    sm2.runOriginal();
    const auto m2 = sm2.getMatching();
    INFO("Weight = ", m2.weight(G));
}

TEST_F(MatcherGTest, testDynamicSuitorMatcher) {
    auto G = NetworkitBinaryReader{}.read("/work/global/angriman/graphs/advogato.nkb");
    if (G.isDirected())
        G = GraphTools::toUndirected(G);
    G.removeSelfLoops();
    G.removeMultiEdges();
    Aux::Random::setSeed(1, false);
    G.forEdges([&](node u, node v) { G.setWeight(u, v, 1.); }); // Aux::Random::real(1, 20)); });
    G.sortOutEdgesByWeight(std::greater<edgeweight>());
    DynamicSuitorMatcher dsm(G);
    dsm.run();
    INFO("Initial weight = ", dsm.getMatching().weight(G));

    static constexpr count updates = 1000;
    std::vector<GraphEvent> batchadditions;
    batchadditions.reserve(updates);
    std::vector<count> prevDegree(G.upperNodeIdBound(), 0);
    G.forNodes([&](const auto u) { prevDegree[u] = G.degree(u); });
    std::vector<count> additionsPerNode(G.upperNodeIdBound(), 0);
    for (count i = 0; i < updates; ++i) {
        node u = none, v = none;
        do {
            u = GraphTools::randomNode(G);
            v = GraphTools::randomNode(G);
        } while (u == v || G.hasEdge(u, v));
        assert(G.indexInOutEdgeArray(u, v) == none);
        assert(G.indexInOutEdgeArray(v, u) == none);
        const GraphEvent ge(GraphEvent::Type::EDGE_ADDITION, u, v, Aux::Random::real(1, 20));
        batchadditions.push_back(ge);
        G.addEdge(ge.u, ge.v, ge.w);
        ++additionsPerNode[u];
        ++additionsPerNode[v];
    }

    dsm.findAffectedAfterEdgeAdditions(batchadditions);
    G.processBatchAdditions(additionsPerNode, updates, dsm.neighborIterators);
    G.forNodes([&](const auto u) {
        std::unordered_set<node> set;
        G.forNeighborsOf(u, [&](const node v) {
            bool found = false;
            for (const auto w : G.neighborRange(v)) {
                if (u == w) {
                    found = true;
                    break;
                }
            }
            assert(found);
            set.insert(v);
        });
        assert(set.size() == G.degree(u));
    });
    dsm.doUpdate();
    INFO("Dynamic update after insertions:   ", dsm.getMatching().weight(G));

    dsm.runOriginal();
    INFO("Static original after insertions:  ", dsm.getMatching().weight(G));
    dsm.run();
    INFO("Static optimized after insertions: ", dsm.getMatching().weight(G));

    G.forNodes([&](const auto u) {
        edgeweight prev = std::numeric_limits<edgeweight>::max();
        G.forNeighborsOf(u, [&](const node v, const edgeweight ew) {
            assert(prev >= ew);
            prev = ew;
        });
    });

    std::vector<index> heaviestRemovals(G.upperNodeIdBound(), none);
    std::vector<GraphEvent> removals;
    removals.reserve(updates);
    // TODO we should use a biased distribution when picking u (proportional to the degree)
    for (count i = 0; i < updates; ++i) {
        node u, v;
        do {
            u = GraphTools::randomNode(G);
            if (G.degree(u) == 0)
                continue;
            index idx = Aux::Random::integer(0, G.degree(u) - 1);
            v = G.getIthNeighbor(u, idx);
        } while (u == v || v == none);
        assert(G.hasEdge(u, v));

        const index uIdx = G.indexInOutEdgeArray(u, v);
        const index vIdx = G.indexInOutEdgeArray(v, u);
        assert(uIdx != none);
        assert(vIdx != none);
        heaviestRemovals[u] = std::min(heaviestRemovals[u], uIdx);
        heaviestRemovals[v] = std::min(heaviestRemovals[v], vIdx);
        G.setHalfEdgeRemoved(u, uIdx);
        G.setHalfEdgeRemoved(v, vIdx);
        removals.emplace_back(GraphEvent::Type::EDGE_REMOVAL, u, v);
    }

    dsm.findAffectedAfterEdgeRemovals(removals);
    count prevM = G.numberOfEdges();
    G.processBatchRemovals(heaviestRemovals, updates, dsm.neighborIterators);
    G.forNodes([&](const auto u) {
        edgeweight prev = std::numeric_limits<edgeweight>::max();
        node prevNode = none;
        G.forNeighborsOf(u, [&](const node v, const edgeweight weight) {
            assert(G.hasEdge(v, u));
            assert(v != none);
            assert(weight <= prev);
            if (weight == prev)
                assert(v < prevNode);
            prev = weight;
            prevNode = v;
        });
    });
    assert(G.numberOfEdges() == prevM - updates);

    dsm.doUpdate();
    auto M = dsm.getMatching();
    G.forEdges([&](const node u, const node v) { assert(M.isMatched(u) || M.isMatched(v)); });
    INFO("Dynamic update after removals:   ", dsm.getMatching().weight(G));
    dsm.runOriginal();
    INFO("Static original after removals:  ", dsm.getMatching().weight(G));
    dsm.run();
    INFO("Static optimized after removals: ", dsm.getMatching().weight(G));
}

TEST_F(MatcherGTest, testLocalMaxMatching) {
    count n = 50;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v); });

    LocalMaxMatcher localMaxMatcher(G);

    TRACE("Start localMax matching");
    localMaxMatcher.run();
    Matching M = localMaxMatcher.getMatching();
    TRACE("Finished localMax matching");

    count numExpEdges = n / 2;
    bool isProper = M.isProper(G);
    EXPECT_TRUE(isProper);
    EXPECT_EQ(M.size(G), numExpEdges);

#if !defined _WIN32 && !defined _WIN64 && !defined WIN32 && !defined WIN64
    DibapGraphReader reader;
    Graph airfoil1 = reader.read("input/airfoil1.gi");
    LocalMaxMatcher lmm(airfoil1);
    lmm.run();
    M = lmm.getMatching();
    isProper = M.isProper(airfoil1);
    EXPECT_TRUE(isProper);
    DEBUG("LocalMax on airfoil1 produces matching of size: ", M.size(G));
#endif
}

TEST_F(MatcherGTest, testLocalMaxMatchingDirectedWarning) {
    Graph G(2, false, true);
    G.addEdge(0, 1);
    EXPECT_THROW(LocalMaxMatcher localMaxMatcher(G), std::runtime_error);
}

TEST_F(MatcherGTest, testPgaMatchingOnWeightedGraph) {
    count n = 50;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v, Aux::Random::real()); });
    PathGrowingMatcher pgaMatcher(G);
    EXPECT_NO_THROW(pgaMatcher.run());
}

TEST_F(MatcherGTest, testPgaMatchingWithSelfLoops) {
    count n = 50;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v, Aux::Random::real()); });
    G.forNodes([&](node u) { G.addEdge(u, u); });
    EXPECT_THROW(PathGrowingMatcher pgaMatcher(G), std::invalid_argument);
}

TEST_F(MatcherGTest, testPgaMatching) {
    count n = 50;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v); });
    PathGrowingMatcher pgaMatcher(G);

    DEBUG("Start PGA matching on 50-clique");

    pgaMatcher.run();
    Matching M = pgaMatcher.getMatching();

    count numExpEdges = n / 2;
    bool isProper = M.isProper(G);
    EXPECT_TRUE(isProper);
    EXPECT_EQ(M.size(G), numExpEdges);
    DEBUG("Finished PGA matching on 50-clique");

#if !defined _WIN32 && !defined _WIN64 && !defined WIN32 && !defined WIN64
    DibapGraphReader reader;
    Graph airfoil1 = reader.read("input/airfoil1.gi");
    PathGrowingMatcher pga2(airfoil1);
    pga2.run();
    M = pga2.getMatching();
    isProper = M.isProper(airfoil1);
    EXPECT_TRUE(isProper);
    DEBUG("PGA on airfoil1 produces matching of size: ", M.size(G));
#endif
}

TEST_F(MatcherGTest, debugValidMatching) {
    METISGraphReader reader;
    Graph G = reader.read("coAuthorsDBLP.graph");

    LocalMaxMatcher pmatcher(G);
    pmatcher.run();
    Matching M = pmatcher.getMatching();

    bool isProper = M.isProper(G);
    EXPECT_TRUE(isProper);
}

} // namespace NetworKit
