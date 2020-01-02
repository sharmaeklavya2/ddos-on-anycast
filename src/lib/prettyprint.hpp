#ifndef PRETTYPRINT_HPP
#define PRETTYPRINT_HPP

#include <iostream>

using std::ostream;
using std::vector;

namespace std {
    template<class T>
    ostream& operator<<(ostream& os, const std::vector<T>& v) {
        os << "[";
        if(!v.empty()) {
            os << v[0];
        }
        for(int i=1; i < int(v.size()); ++i) {
            os << ", " << v[i];
        }
        os << "]";
        return os;
    }
}

#endif  // PRETTYPRINT_HPP
