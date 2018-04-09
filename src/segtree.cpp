#include "lib/segtree.hpp"
#include <cstring>

int main(int argc, char* argv[]) {
    if(argc == 1) {
        fprintf(stderr, "usage: %s space_separated_list_of_numbers\n", argv[0]);
        return 2;
    }

    int n = argc - 1;
    vector<double> v(n);
    for(int i=1; i<argc; ++i) {
        v[i-1] = std::atof(argv[i]);
    }

    SegTree segtree(v);

    printf("commands: print, query i, update i x, cumsum_search x;  Ctrl+D to quit\n");

    char command[10];
    while(true) {
        printf(">> ");
        if(scanf("%s", command) == EOF) {
            printf("\r");
            break;
        }
        if(command[0] == 'p') {
            segtree.print(stdout);
        }
        else if(command[0] == 'u') {
            int i; double x;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
            scanf("%d%lf", &i, &x);
#pragma GCC diagnostic pop
            segtree.update(i, x);
        }
        else if(command[0] == 'q') {
            int i; double x;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
            scanf("%d", &i);
#pragma GCC diagnostic pop
            x = segtree.query(i);
            printf("%lf\n", x);
        }
        else if(command[0] == 'c') {
            int i; double x;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
            scanf("%lf", &x);
#pragma GCC diagnostic pop
            i = segtree.cumsum_search(x);
            printf("%d\n", i);
        }
        else {
            printf("Invalid command\n");
        }
    };
    return 0;
}
