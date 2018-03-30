#include "dag.hpp"
#include "util.hpp"

void Graph::get_sources_and_sinks(ivec& sources, ivec& sinks) const {
    for(int u=0; u<n; ++u) {
        if(_preds[u].empty()) {
            sources.push_back(u);
        }
        if(_succs[u].empty()) {
            sinks.push_back(u);
        }
    }
}

void Graph::print(FILE* fp) const {
    for(int u=0; u<n; ++u) {
        fprintf(fp, "%d: ", u);
        print_icont(fp, _succs[u], ", ", "[", "] ");
        print_icont(fp, _preds[u], ", ", "[", "]\n");
    }
}

class DFSHelper {
public:
    const Graph& graph;
    vector<bool> visited;
    ivec& topolist;

    DFSHelper(const Graph& _graph, ivec& _topolist):
        graph(_graph), visited(_graph.order(), false), topolist(_topolist) {}

    void do_dfs(int u);
};

void DFSHelper::do_dfs(int u) {
    if(visited[u]) {
        return;
    }
    visited[u] = true;
    for(int v: graph.succs(u)) {
        if(!visited[v]) {
            do_dfs(v);
        }
    }
    topolist.push_back(u);
}

void rtoposort(const Graph& graph, ivec& output) {
    int n = graph.order();
    output.reserve(n);
    DFSHelper dfs_helper(graph, output);
    for(int u=0; u<n; ++u) {
        dfs_helper.do_dfs(u);
    }
}

void toposort(const Graph& graph, ivec& output) {
    rtoposort(graph, output);
    std::reverse(output.begin(), output.end());
}
