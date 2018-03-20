#include "lib/graph_gen.hpp"

using std::atoi;

int main(int argc, char* argv[]) {
    char args_str[] = "n m seed";
    int n, m, seed=-1;
    if(argc <= 2 || argc >= 5) {
        fprintf(stderr, "usage: %s %s\n", argv[0], args_str);
        return 2;
    }
    else {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if(argc == 4) {
            seed = atoi(argv[3]);
        }
    }
    if(seed == -1) {
        seed = int(time(nullptr));
    }

    ipvec l;

    make_random_connected_graph(n, m, l, seed);
    for(int i=0; i<n; ++i) {
        printf("%d\n", i);
    }

    for(const iipair& p: l) {
        int u = p.first, v = p.second;
        printf("%d %d\n", u, v);
    }
    return 0;
}
