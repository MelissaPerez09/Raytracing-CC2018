#pragma once

#include <string>
#include <SDL.h>
#include <glm/glm.hpp>

class Skybox {
public:
    Skybox(const std::string& textureFile);
    ~Skybox();

    glm::vec3 getColor(const glm::vec3& direction) const;

private:
    SDL_Surface* texture;

    void loadTexture(const std::string& textureFile);
};