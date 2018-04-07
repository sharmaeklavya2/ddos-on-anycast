#include "lib/dag.hpp"

int main() {
    int n, m;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    scanf("%d%d", &n, &m);
#pragma GCC diagnostic pop
    Graph graph(n);
    for(int i=0; i<m; ++i) {
        int u, v;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
        scanf("%d%d", &u, &v);
#pragma GCC diagnostic pop
        graph.add_edge(u, v);
    }
    ivec topolist;
    toposort(graph, topolist);
    printf("%d", topolist[0]);
    for(int i=1; i<n; ++i) {
        printf(" %d", topolist[i]);
    }
    printf("\n");
    return 0;
}
