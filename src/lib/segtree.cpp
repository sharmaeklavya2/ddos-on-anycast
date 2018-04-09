#include "segtree.hpp"

static segtree_value_type segtree_zero = 0;
static inline segtree_value_type segtree_join(segtree_value_type a, segtree_value_type b) {return a+b;}


void SegTree::_init(int root, int first, int last, const value_type* a) {
    if(first == last) {
        v[root] = a[first];
    }
    else {
        int mid = (first + last) / 2;
        int left = 2 * root + 1;
        _init(left, first, mid, a);
        _init(left+1, mid+1, last, a);
        v[root] = segtree_join(v[left], v[left+1]);
    }
}


void SegTree::_print(FILE* fp, int root, int first, int last, int indent) const {
    for(int i=0; i<indent; ++i) {
        fprintf(fp, "  ");
    }
    fprintf(fp, "%d(%d-%d): %lf\n", root, first, last, v[root]);
    if(first != last) {
        int mid = (first + last) / 2;
        int left = 2 * root + 1;
        _print(fp, left, first, mid, indent+1);
        _print(fp, left+1, mid+1, last, indent+1);
    }
}


typename SegTree::value_type SegTree::_query(int root, int first, int last, int index) const {
    if(index < first || index > last) {
        return segtree_zero;
    }
    else if(first == last) {
        return v[root];
    }
    else {
        int mid = (first+last) / 2;
        int left = 2 * root + 1;
        return _query(left, first, mid, index) + _query(left+1, mid+1, last, index);
    }
}


void SegTree::_update(int root, int first, int last, int index, value_type x) {
    if(index < first || index > last) {
        return;
    }
    else if(first == last) {
        v[root] = x;
    }
    else {
        int mid = (first + last) / 2;
        int left = 2 * root + 1;
        _update(left, first, mid, index, x);
        _update(left+1, mid+1, last, index, x);
        v[root] = segtree_join(v[left], v[left+1]);
    }
}


int SegTree::_cumsum_search(int root, int first, int last, value_type x) const {
    if(x > v[root]) {
        return -1;
    }
    else if(first == last) {
        return first;
    }
    int left = 2 * root + 1;
    int mid = (first + last) / 2;
    if(x <= v[left]) {
        return _cumsum_search(left, first, mid, x);
    }
    else {
        return _cumsum_search(left+1, mid+1, last, x-v[left]);
    }
}
