#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "graph_gen.hpp"

struct GrowParams {
    int num_children;
    double num_children_sigma;
    double prob_multi_upstream;
    double prob_self_multi_upstream;
    double prob_side_peering;
    double prob_self_side_peering;
    GraphGen* p_explode_graph_gen;
};

class Network {
public:
    /*
    A network has two types of nodes, physical and virtual.
    A virtual node is a node which has no physical existence of its own. It represents a subnetwork.
    */

    typedef ivec nbrs_t;
    typedef vector<nbrs_t> nbrs_list_t;

    ivec nid;
    // node number of the virtual node representing the network this node is part of.
    // a virtual node can itself be part of a higher network.

    ivec netsize;
    // netsize[u] is 0 iff u is a physical node
    // Otherwise netsize[u] is the number of nodes in the network belonging to the virtual node u.
    ivec depth;
    int height;
    int pivot;  // index of a physical node

    void update_pivot() {
        for(int i=0; i<int(netsize.size()); ++i) {
            if(netsize[i] == 0) {
                pivot = i;
                break;
            }
        }
    }

    nbrs_list_t in_nbrs;
    nbrs_list_t side_nbrs;
    nbrs_list_t up_nbrs;
    nbrs_list_t down_nbrs;
    /*
    Each node has 4 kinds of neighbors:
    * in_nbrs: neighbors within the local network.
    * side_nbrs: neighbors in a different network at the same level.
    * up_nbrs: neighbors in the upstream network.
    * down_nbrs: neighbors in the downstream network.
    A virtual node doesn't have in_nbrs and down_nbrs.
    */

    void init(int n, const ipvec& edges, bool virtual_parent, int side_connectivity);

    Network() {
        init(1, ipvec(), false, 0);
    }
    Network(int n, const ipvec& edges, bool virtual_parent, int side_connectivity) {
        init(n, edges, virtual_parent, side_connectivity);
    }
    Network(GraphGen& graph_gen, int seed, bool virtual_parent, int side_connectivity) {
        ipvec edges;
        int n = graph_gen(edges, seed);
        init(n, edges, virtual_parent, side_connectivity);
    }

    // properties

    int degree(int u) const {
        return in_nbrs[u].size() + side_nbrs[u].size() + up_nbrs[u].size() + down_nbrs[u].size();
    }
    bool is_virtual(int u) const {
        return netsize[u] > 0;
    }

    int size() const {
        // returns the number of physical and virtual nodes
        return netsize.size();
    }

    int num_vertices() const;
    // returns the number of physical nodes

    int num_edges() const;
    // returns the number of edges between physical nodes

    ipvec edge_list() const;

    void print(ostream& os) const;
    void print(FILE* fp, bool print_virtual) const;

    bool basic_sanity_check() const;
    bool long_sanity_check() const;

    // operations

    void add_in_edge(int u, int v);
    void add_side_edge(int u, int v);
    void add_upward_edge(int u, int v);

    int make_new_vertices(int k, int _nid);
    // insert k new physical nodes in the network whose network id is _nid

    void grow(int u, int level, bool fully_expand_last_level, const GrowParams& grow_params, int seed);
    // replace a physical vertex u by a network
};

ostream& operator<<(ostream&, const Network&);

#endif  // NETWORK_HPP
