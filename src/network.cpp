#include "lib/network.hpp"
#include "lib/graph_gen.hpp"

int main() {
    int retval = 0;

    RandomConnectedGraphGen rcgen(8, 3.0);
    NormalOrderGraphGen explode_graph_gen(0.25, rcgen);

    GrowParams grow_params = {
        8, // num_children
        0.25, // num_children_sigma
        0.1, // prob_multi_upstream
        0.3, // prob_self_multi_upstream
        0.1, // prob_side_peering
        0.8, // prob_self_side_peering
        &explode_graph_gen,
    };

    int seed;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    scanf("%d", &seed);
#pragma GCC diagnostic pop
    Network network;
    if(!network.long_sanity_check()) {
        puts("Network has inconsistencies!");
        retval = 1;
    }
    printf("Vertices: %d\n", network.num_vertices());
    printf("Edges: %d\n", network.num_edges());
    printf("\n");

    network.grow(0, 3, false, grow_params, seed);
    printf("\n");

    if(!network.long_sanity_check()) {
        puts("Network has inconsistencies!");
        retval = 1;
    }
    printf("Vertices: %d\n", network.num_vertices());
    printf("Edges: %d\n", network.num_edges());
    printf("\n");

    network.print(stdout, false);

    // ipvec l = network.edge_list();
    // for(const iipair& p: l) {
        // printf("%d %d\n", p.first, p.second);
    // }
    return retval;
}
