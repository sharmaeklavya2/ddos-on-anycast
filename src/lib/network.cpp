#include "network.hpp"
#include "util.hpp"
#include "graph_gen.hpp"

void Network::init(int n, const ipvec& edges, bool virtual_parent, int side_connectivity) {
    // if side_connectivity is positive, that many side_links are made for each edge
    // otherwise in_links are used.
    int vparent = -1;
    depthwise.resize(1);
    if(virtual_parent) {
        vparent = 0;
        make_new_vertices(1, -1);
        depthwise[0].push_back(0);
    }
    int ufirst = vparent + 1;
    int uend = ufirst + n;
    make_new_vertices(n, vparent);
    for(int i=ufirst; i<uend; ++i) {
        depthwise[0].push_back(i);
    }

    for(const iipair& p: edges) {
        int u = ufirst + p.first, v = ufirst + p.second;
        if(side_connectivity > 0) {
            for(int i=0; i<side_connectivity; ++i) {
                add_side_edge(u, v);
            }
        }
        else {
            add_in_edge(u, v);
        }
    }
}

void Network::add_in_edge(int u, int v) {
    insert(in_nbrs[u], v);
    insert(in_nbrs[v], u);
}
void Network::add_side_edge(int u, int v) {
    insert(side_nbrs[u], v);
    insert(side_nbrs[v], u);
}
void Network::add_upward_edge(int u, int v) {
    insert(up_nbrs[u], v);
    insert(down_nbrs[v], u);
    if(is_virtual(v)) {
        fprintf(stderr, "virtual node %d got a child %d\n", v, u);
    }
    if(depth[u] != depth[v] + 1) {
        fprintf(stderr, "Depth of lower node %d is %d, depth of upper node %d is %d\n",
            u, depth[u], v, depth[v]);
    }
}

int Network::num_vertices() const {
    // returns the number of physical nodes
    int n = 0;
    for(int s: netsize) {
        if(s == 0) {
            n++;
        }
    }
    return n;
}

int Network::num_edges() const {
    // returns the number of edges between physical nodes
    int m = 0;
    for(int u=0; u<size(); ++u) {
        if(netsize[u] == 0) {
            m += degree(u);
        }
    }
    if(m % 2 != 0) {
        fprintf(stderr, "Degree sum is odd.\n");
    }
    return m / 2;
}

ipvec Network::edge_list() const {
    ipvec l;
    for(int u=0; u<size(); ++u) {
        if(!is_virtual(u)) {
            for(int v: in_nbrs[u]) {if(v > u) {l.push_back(iipair(u, v));}}
            for(int v: side_nbrs[u]) {if(v > u) {l.push_back(iipair(u, v));}}
            for(int v: up_nbrs[u]) {if(v > u) {l.push_back(iipair(u, v));}}
            for(int v: down_nbrs[u]) {if(v > u) {l.push_back(iipair(u, v));}}
        }
    }
    return l;
}

int Network::make_new_vertices(int k, int _nid) {
    // insert k new physical nodes in the network whose network id is _nid
    // doesn't fill 'depthwise'
    int u = size();
    int uend = u+k;
    nid.resize(uend, _nid);
    netsize.resize(uend, 0);
    if(_nid >= 0) {
        netsize[_nid] += k;
        depth.resize(uend, depth[_nid]);
    }
    else {
        depth.resize(uend, 0);
    }

    in_nbrs.resize(uend);
    side_nbrs.resize(uend);
    up_nbrs.resize(uend);
    down_nbrs.resize(uend);
    return u;
}

void Network::print(ostream& os) const {
    os << "Network"
        << "(\n        nid = " << nid
        << ",\n    netsize = " << netsize
        << "(\n      depth = " << depth
        << ",\n    in_nbrs = " << in_nbrs
        << ",\n  side_nbrs = " << side_nbrs
        << ",\n    up_nbrs = " << up_nbrs
        << ",\n  down_nbrs = " << down_nbrs
        << "\n)\n";
}

void Network::print(FILE* fp, bool print_virtual) const {
    for(int u=0; u<size(); ++u) {
        if(print_virtual || netsize[u] == 0) {
            fprintf(fp, "%d:\n  nid: %d, netsize: %d, depth: %d,\n", u, nid[u], netsize[u], depth[u]);
            fprintf(fp, "  in_nbrs: ");
            print_icont(fp, in_nbrs[u], ", ", "[", "],\n");
            fprintf(fp, "  side_nbrs: ");
            print_icont(fp, side_nbrs[u], ", ", "[", "],\n");
            fprintf(fp, "  up_nbrs: ");
            print_icont(fp, up_nbrs[u], ", ", "[", "],\n");
            fprintf(fp, "  down_nbrs: ");
            print_icont(fp, down_nbrs[u], ", ", "[", "]\n");
        }
    }
    fprintf(fp, "depthwise:\n");
    for(size_t i=0; i<depthwise.size(); ++i) {
        fprintf(fp, "  %zd: ", i);
        print_icont(fp, depthwise[i], ", ", "[", "]\n");
    }
}

namespace std {
    ostream& operator<<(ostream& os, const Network& network) {
        network.print(os);
        return os;
    }
}

bool Network::basic_sanity_check() const {
    // sizes of all lists should be the same
    size_t sizes[7] = {nid.size(), depth.size(), netsize.size(),
        in_nbrs.size(), side_nbrs.size(), up_nbrs.size(), down_nbrs.size()};
    for(int i=0; i<6; ++i) {
        if(sizes[i] != sizes[i+1]) {
            fprintf(stderr, "sizes of lists %d and %d differ\n", i, i+1);
            return false;
        }
    }
    int s = 0;
    for(int i=0; i<=height(); ++i) {
        s += depthwise[i].size();
    }
    if(s != size()) {
        fprintf(stderr, "depthwise node-count sum doesn't equal total node-count\n");
        return false;
    }
    return true;
}

bool Network::long_sanity_check() const {
    if(!basic_sanity_check()) {
        return false;
    }
    int s = 0;

    typedef const nbrs_list_t* pnbrs_list_t;
    pnbrs_list_t nbrs_ptrs[] = {&in_nbrs, &side_nbrs, &up_nbrs, &down_nbrs};
    const char* nbrs_names[] = {"in_nbrs", "side_nbrs", "up_nbrs", "down_nbrs"};

    // consistency of nid and netsize
    ivec netsize2(size(), 0);
    for(int vnode: nid) {
        if(vnode >= 0) {
            netsize2[vnode]++;
        }
    }

    int height2 = 0;
    for(int u=0; u<size(); ++u) {
        if(netsize[u] == 0 && depth[u] > height2) {
            height2 = depth[u];
        }
    }
    if(height2 != height()) {
        fprintf(stderr, "height should be %d instead of %d\n", height2, height());
        return false;
    }

    for(int i=0; i<=height(); ++i) {
        for(int u: depthwise[i]) {
            if(depth[u] != i) {
                fprintf(stderr, "%d is in depthwise[%d] but has depth %d\n", u, i, depth[u]);
            }
        }
    }

    for(int u=0; u<size(); ++u) {

        // consistency of nid and netsize
        if(netsize[u] != netsize2[u]) {
            fprintf(stderr, "netsize[%d] is %d instead of %d\n", u, netsize[u], netsize2[u]);
            return false;
        }

        // no self-loops are allowed
        for(int i=0; i<4; ++i) {
            for(int v: (*(nbrs_ptrs[i]))[u]) {
                if(v == u) {
                    fprintf(stderr, "%s[%d] contains itself\n", nbrs_names[i], u);
                    return false;
                }
            }
        }

        if(is_virtual(u)) {
            // virtual node shouldn't have any in_nbrs
            if(in_nbrs[u].size() > 0) {
                fprintf(stderr, "virtual node %d has in_nbrs ", u);
                cerr << in_nbrs[u] << endl;
                return false;
            }
            /*
            else if(down_nbrs[u].size() > 0) {
                fprintf(stderr, "virtual node %d has down_nbrs ", u);
                cerr << down_nbrs[u] << endl;
                return false;
            }
            */
        }
        else {
            s += degree(u);
            // Network should be symmetric because it is undirected
            for(int v: in_nbrs[u]) {
                if(find(in_nbrs[v], u) == in_nbrs[v].end()) {
                    fprintf(stderr, "in_nbrs[%d] has %d but in_nbrs[%d] doesn't have %d\n", u, v, v, u);
                    return false;
                }
                if(is_virtual(v)) {
                    fprintf(stderr, "in_nbrs[%d] has virtual node %d\n", u, v);
                    return false;
                }
            }
            for(int v: side_nbrs[u]) {
                if(find(side_nbrs[v], u) == side_nbrs[v].end()) {
                    fprintf(stderr, "side_nbrs[%d] has %d but side_nbrs[%d] doesn't have %d\n", u, v, v, u);
                    return false;
                }
                if(is_virtual(v)) {
                    fprintf(stderr, "side_nbrs[%d] has virtual node %d\n", u, v);
                    return false;
                }
                /*
                int nid_u = nid[u];
                if(nid_u >= 0) {
                    if(find(side_nbrs[nid_u], v) == side_nbrs[nid_u].end()) {
                        fprintf(stderr, "%d has a side_nbr %d but %d's nid doesn't\n", u, v, u);
                        return false;
                    }
                }
                */
            }
            for(int v: up_nbrs[u]) {
                if(find(down_nbrs[v], u) == down_nbrs[v].end()) {
                    fprintf(stderr, "up_nbrs[%d] has %d but down_nbrs[%d] doesn't have %d\n", u, v, v, u);
                    return false;
                }
                if(is_virtual(v)) {
                    fprintf(stderr, "up_nbrs[%d] has virtual node %d\n", u, v);
                    return false;
                }
                /*
                int nid_u = nid[u];
                if(nid_u >= 0) {
                    if(find(up_nbrs[nid_u], v) == up_nbrs[nid_u].end()) {
                        fprintf(stderr, "%d has a up_nbr %d but %d's nid doesn't\n", u, v, u);
                        return false;
                    }
                }
                */
            }
            for(int v: down_nbrs[u]) {
                if(find(up_nbrs[v], u) == up_nbrs[v].end()) {
                    fprintf(stderr, "down_nbrs[%d] has %d but up_nbrs[%d] doesn't have %d\n", u, v, v, u);
                    return false;
                }
                if(is_virtual(v)) {
                    fprintf(stderr, "down_nbrs[%d] has virtual node %d\n", u, v);
                    return false;
                }
            }
        }
    }
    if(s % 2 != 0) {
        fprintf(stderr, "Degree sum is odd.\n");
        return false;
    }
    return true;
}
