#ifndef DAG_HPP
#define DAG_HPP

#include "base.hpp"

class Graph {
    int n;

    vector<ivec> _preds, _succs;

public:
    explicit Graph(int _n): n(_n), _preds(_n), _succs(_n) {}

    int order() const {return n;}
    const ivec& preds(int u) const {return _preds[u];}
    const ivec& succs(int u) const {return _succs[u];}

    void add_edge(int u, int v) {
        _succs[u].push_back(v);
        _preds[v].push_back(u);
    }

    void get_sources_and_sinks(ivec& sources, ivec& sinks) const;

    void print(FILE* fp) const;

    void swap(Graph& graph) {
        std::swap(n, graph.n);
        std::swap(_preds, graph._preds);
        std::swap(_succs, graph._succs);
    }
};

void rtoposort(const Graph& graph, ivec& output);
void toposort(const Graph& graph, ivec& output);

#endif  // DAG_HPP
