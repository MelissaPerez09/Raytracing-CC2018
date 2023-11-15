#include "skybox.h"
#include "SDL_image.h"
#include <iostream>

Skybox::Skybox(const std::string& textureFile) {
    texture = nullptr;
    loadTexture(textureFile);
}

Skybox::~Skybox() {
    SDL_FreeSurface(texture);
}

void Skybox::loadTexture(const std::string& textureFile) {
    texture = IMG_Load(textureFile.c_str());
    if (!texture) {
        throw std::runtime_error("Failed to load skybox texture: " + std::string(IMG_GetError()));
    }
}

glm::vec3 Skybox::getColor(const glm::vec3& direction) const {

    if (!texture) {
        return glm::vec3(0.5f, 0.7f, 1.0f); // Default color if texture is not loaded
    }

    // Map direction to spherical coordinates
    float phi = atan2(direction.z, direction.x);
    float theta = asin(direction.y);

    // Map spherical coordinates to UV coordinates
    const float pi = 3.14159265358979323846f;
    float u = 0.5f + phi / (2.0f * pi);
    float v = 0.5f - theta / pi;

    // Sample the texture
    int texX = static_cast<int>(u * texture->w);
    int texY = static_cast<int>(v * texture->h);

    // Ensure coordinates are within bounds
    texX = glm::clamp(texX, 0, texture->w - 1);
    texY = glm::clamp(texY, 0, texture->h - 1);

    // Get the color from the texture
    Uint8* pixels = static_cast<Uint8*>(texture->pixels);
    Uint8* pixel = pixels + texY * texture->pitch + texX * texture->format->BytesPerPixel;

    float colorR = pixel[0] / 255.0f;
    float colorG = pixel[1] / 255.0f;
    float colorB = pixel[2] / 255.0f;

    return glm::vec3(colorR, colorG, colorB);
}
