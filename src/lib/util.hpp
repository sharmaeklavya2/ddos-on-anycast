#ifndef UTIL_HPP
#define UTIL_HPP

#include "base.hpp"

// Utility functions

MAYBE_INLINE ivec::const_iterator find(const ivec& v, int x) {
    return std::find(v.cbegin(), v.cend(), x);
}

MAYBE_INLINE imset::const_iterator find(const imset& s, int x) {
    return s.find(x);
}

MAYBE_INLINE void insert(ivec& v, int x) {v.push_back(x);}
MAYBE_INLINE void insert(imset& s, int x) {s.insert(x);}

template<class C>
inline bool find_and_erase(C& c, typename C::value_type x) {
    typename C::const_iterator it = find(c, x);
    if(it == c.end()) {
        return false;
    }
    else {
        c.erase(it);
        return true;
    }
}

#endif  // UTIL_HPP
