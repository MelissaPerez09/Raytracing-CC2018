#pragma once

#include "glm/glm.hpp"
#include "color.h"

struct Light {
    glm::vec3 position;
    float intensity;
    Color color;

    // Constructor with three arguments
    Light(const glm::vec3& pos, float intensity, const Color& col)
        : position(pos), intensity(intensity), color(col) {}
};
