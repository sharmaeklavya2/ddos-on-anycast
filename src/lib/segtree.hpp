#ifndef SEGTREE_HPP
#define SEGTREE_HPP

#include "base.hpp"

typedef double segtree_value_type;

class SegTree {
public:
    typedef segtree_value_type value_type;

    int n;
    vector<value_type> v;
    explicit SegTree(int _n): n(_n), v(2*_n) {}
    explicit SegTree(const vector<value_type>& _v): n(_v.size()), v(4*_v.size()) {
        init(_v.data());
    }

    void _init(int root, int first, int last, const value_type* a);
    void init(const value_type* a) {_init(0, 0, n-1, a);}

    void _print(FILE* fp, int root, int first, int last, int indent) const;
    void print(FILE* fp) const {_print(fp, 0, 0, n-1, 0);}

    value_type _query(int root, int first, int last, int index) const;
    value_type query(int index) const {return _query(0, 0, n-1, index);}

    void _update(int root, int first, int last, int index, value_type x);
    void update(int index, value_type x) {_update(0, 0, n-1, index, x);}

    int _cumsum_search(int root, int first, int last, value_type sum) const;
    int cumsum_search(value_type sum) const {return _cumsum_search(0, 0, n-1, sum);}

    value_type get_root() const {return v[0];}
};

#endif  // SEGTREE_HPP
