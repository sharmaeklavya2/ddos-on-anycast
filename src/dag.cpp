#include "lib/dag.hpp"

int main() {
    int n, m;
    scanf("%d%d", &n, &m);
    Graph graph(n);
    for(int i=0; i<m; ++i) {
        int u, v;
        scanf("%d%d", &u, &v);
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
