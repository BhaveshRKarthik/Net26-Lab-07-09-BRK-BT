#pragma once

#include <array>
#include <cstring>
#include "config.hpp"

struct Neighbourhood {
    std::array<int, 26*26> adj;

    Neighbourhood() {
        adj.fill(-1);
    }

    auto& operator[](this auto& self, int i, int j) {
        return self.adj[26 * i + j];
    }
};