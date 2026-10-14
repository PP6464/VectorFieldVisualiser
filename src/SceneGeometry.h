#pragma once

#include <cstdint>
#include <vector>

#include "Vertex.h"

namespace vf::scene
{
    /// Represents the entities to be rendered
    struct geometry
    {
        std::vector<vertex::vertex> axes;
        std::vector<vertex::vertex> ticks;
        std::vector<vertex::vertex> axis_arrows;
        std::vector<std::uint32_t> axis_arrow_indices;
        std::vector<vertex::vertex> field_arrows;
    };

    /// Configure the axes, ticks and arrow lengths
    geometry create_geometry(int axes_size = 5, int stride = 2, float arrow_length = 0.5f);
}
