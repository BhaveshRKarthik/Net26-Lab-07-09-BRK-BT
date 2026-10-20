#pragma once

#include <array>
#include <cstring>
#include "config.hpp"

struct Neighbourhood {
    std::array<int, 26*26> adj;

    Neighbourhood() {
        adj.fill(-1);
    }

    int& operator[](int i, int j) {
        return adj[26 * i + j];
    }
    
    const int& operator[](int i, int j) const {
        return adj[26 * i + j];
    }
};