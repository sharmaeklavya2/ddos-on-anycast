#ifndef GRAPH_GEN_HPP
#define GRAPH_GEN_HPP

#include "base.hpp"

void make_cycle_graph(int n, ipvec& l);
void make_complete_graph(int n, ipvec& l);
void make_random_connected_graph(int n, int approx_m, ipvec& l, int seed);

class GraphGen {
public:
    int n;
    explicit GraphGen(int _n): n(_n) {}
    virtual int operator()(ipvec& l, int seed) = 0;
};

class CycleGraphGen: public GraphGen {
public:
    explicit CycleGraphGen(int _n): GraphGen(_n) {}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    virtual int operator()(ipvec& l, int seed) {
        make_cycle_graph(n, l);
        return n;
    }
#pragma GCC diagnostic pop
};

class CompleteGraphGen: public GraphGen {
public:
    explicit CompleteGraphGen(int _n): GraphGen(_n) {}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    virtual int operator()(ipvec& l, int seed) {
        make_complete_graph(n, l);
        return n;
    }
#pragma GCC diagnostic pop
};

class RandomConnectedGraphGen: public GraphGen {
public:
    double avg_degree;
    RandomConnectedGraphGen(int _n, double _avg_degree):
        GraphGen(_n), avg_degree(_avg_degree) {}
    virtual int operator()(ipvec& l, int seed) {
        make_random_connected_graph(n, int(n * avg_degree / 2), l, seed);
        return n;
    }
};

class NormalOrderGraphGen: public GraphGen {
public:
    double sigma;
    GraphGen& graph_gen;
    // this.n stores the average value of the number of nodes
    // graph_gen.n stores the number of nodes generated when operator() was last called
    explicit NormalOrderGraphGen(double _sigma, GraphGen& _graph_gen):
        GraphGen(_graph_gen.n), sigma(_sigma), graph_gen(_graph_gen) {}
        // It's assumed that graph_gen isn't stateful wrt graph_gen.n
    virtual int operator()(ipvec& l, int seed) {
        rng_t rng(seed);
        rng_t rng2(rng());
        graph_gen.n = std::max(1, int(n * std::normal_distribution<double>(1.0, sigma)(rng2)));
        return graph_gen(l, rng());
    }
};

#endif  // GRAPH_GEN_HPP
