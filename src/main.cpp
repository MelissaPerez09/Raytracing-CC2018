#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <print.h>

#include "color.h"
#include "intersect.h"
#include "object.h"
#include "sphere.h"
#include "light.h"
#include "camera.h"
#include "cube.h"
#include "skybox.h"

Skybox skybox("../assets/skybox.png");
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float ASPECT_RATIO = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
const int MAX_RECURSION = 3;
const float BIAS = 0.0001f;

SDL_Renderer* renderer;
std::vector<Object*> objects;
Light light(glm::vec3(-1.0, 0, 10), 1.5f, Color(255, 255, 255));
Camera camera(glm::vec3(0.0, 0.0, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 10.0f);


void point(glm::vec2 position, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, position.x, position.y);
}

float castShadow(const glm::vec3& shadowOrigin, const glm::vec3& lightDir, Object* hitObject) {
    for (auto& obj : objects) {
        if (obj != hitObject) {
            Intersect shadowIntersect = obj->rayIntersect(shadowOrigin, lightDir);
            if (shadowIntersect.isIntersecting && shadowIntersect.dist > 0) {
                float shadowRatio = shadowIntersect.dist / glm::length(light.position - shadowOrigin);
                shadowRatio = glm::min(1.0f, shadowRatio);
                return 1.0f - shadowRatio;
            }
        }
    }
    return 1.0f;
}

Color castRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const short recursion = 0) {
    float zBuffer = 99999;
    Object* hitObject = nullptr;
    Intersect intersect;

    for (const auto& object : objects) {
        Intersect i = object->rayIntersect(rayOrigin, rayDirection);
        if (i.isIntersecting && i.dist < zBuffer) {
            zBuffer = i.dist;
            hitObject = object;
            intersect = i;
        }
    }

    if (!intersect.isIntersecting || recursion == MAX_RECURSION) {
        glm::vec3 skyboxColor = skybox.getColor(rayDirection);
        return Color(skyboxColor.r, skyboxColor.g, skyboxColor.b); 
    }


    glm::vec3 lightDir = glm::normalize(light.position - intersect.point);
    glm::vec3 viewDir = glm::normalize(rayOrigin - intersect.point);
    glm::vec3 reflectDir = glm::reflect(-lightDir, intersect.normal); 

    float shadowIntensity = castShadow(intersect.point, lightDir, hitObject);

    float diffuseLightIntensity = std::max(0.0f, glm::dot(intersect.normal, lightDir));
    float specReflection = glm::dot(viewDir, reflectDir);
    
    Material mat = hitObject->material;

    float specLightIntensity = std::pow(std::max(0.0f, glm::dot(viewDir, reflectDir)), mat.specularCoefficient);


    Color reflectedColor(0.0f, 0.0f, 0.0f);
    if (mat.reflectivity > 0) {
        glm::vec3 origin = intersect.point + intersect.normal * BIAS;
        reflectedColor = castRay(origin, reflectDir, recursion + 1); 
    }

    Color refractedColor(0.0f, 0.0f, 0.0f);
    if (mat.transparency > 0) {
        glm::vec3 origin = intersect.point - intersect.normal * BIAS;
        glm::vec3 refractDir = glm::refract(rayDirection, intersect.normal, mat.refractionIndex);
        refractedColor = castRay(origin, refractDir, recursion + 1); 
    }

    Color diffuseLight = mat.diffuse * light.intensity * diffuseLightIntensity * mat.albedo * shadowIntensity;
    Color specularLight = light.color * light.intensity * specLightIntensity * mat.specularAlbedo * shadowIntensity;
    Color color = (diffuseLight + specularLight) * (1.0f - mat.reflectivity - mat.transparency) + reflectedColor * mat.reflectivity + refractedColor * mat.transparency;
    return color;
} 

void setUpBee() {
    // BEE MAIN BODY
    Material yellow1 = {
        Color(172, 154, 82),
        1.0,
        0.0,
        9.0f,
        0.0f,
        0.0f
    };

    Material yellow2 = {
        Color(164, 140, 53),
        1.0,
        0.0,
        9.0f,
        0.0f,
        0.0f
    };

    Material yellow3 = {
        Color(152, 123, 39),
        1.0,
        0.0,
        9.0f,
        0.0f,
        0.0f
    };

    glm::vec3 bigCubePosition(0.0f, 0.0f, 0.0f); // Center at (0, 0, 0)
    float bigCubeSideLength = 1.5f;
    int gridResolution = 5;

    float smallCubeSideLength = bigCubeSideLength / gridResolution;
    std::srand(std::time(0));

    // Create the grid of smaller cubes
    for (int x = 0; x < gridResolution; ++x) {
        for (int y = 0; y < gridResolution; ++y) {
            for (int z = 0; z < gridResolution; ++z) {
                glm::vec3 smallCubePosition = bigCubePosition + glm::vec3(x * smallCubeSideLength, y * smallCubeSideLength, z * smallCubeSideLength);

                int materialIndex = std::rand() % 3;
                Material cubeMaterial;

                switch (materialIndex) {
                    case 0:
                        cubeMaterial = yellow1;
                        break;
                    case 1:
                        cubeMaterial = yellow2;
                        break;
                    case 2:
                        cubeMaterial = yellow3;
                        break;
                }
                objects.push_back(new Cube(smallCubePosition, smallCubeSideLength, cubeMaterial));
            }
        }
    }

    //WINGS
    Material gray1 = {
        Color(112, 116, 117),
        1.0,
        0.0,
        9.0f,
        0.1f,
        0.3f
    };

    Material gray2 = {
        Color(172, 175, 172),
        1.0,
        0.0,
        9.0f,
        0.1f,
        0.3f
    };

    Material gray3 = {
        Color(122, 125, 130),
        1.0,
        0.0,
        9.0f,
        0.1f,
        0.3f
    };

   //left
    objects.push_back(new Cube(glm::vec3(0.0f, 2.0f, 0.5f), 0.3f, gray1));
    objects.push_back(new Cube(glm::vec3(0.0f, 2.0f, 0.5f), 0.3f, gray2));
    objects.push_back(new Cube(glm::vec3(0.3f, 1.8f, 0.5f), 0.3f, gray2));
    objects.push_back(new Cube(glm::vec3(0.3f, 1.8f, 0.5f), 0.3f, gray3));
    objects.push_back(new Cube(glm::vec3(0.4f, 1.5f, 0.5f), 0.3f, gray1));
    objects.push_back(new Cube(glm::vec3(0.4f, 1.5f, 0.5f), 0.3f, gray3));

    //right
    objects.push_back(new Cube(glm::vec3(1.3f, 2.0f, 0.5f), 0.3f, gray2));
    objects.push_back(new Cube(glm::vec3(1.3f, 2.0f, 0.5f), 0.3f, gray3));
    objects.push_back(new Cube(glm::vec3(1.0f, 1.8f, 0.5f), 0.3f, gray1));
    objects.push_back(new Cube(glm::vec3(1.0f, 1.8f, 0.5f), 0.3f, gray2));
    objects.push_back(new Cube(glm::vec3(0.9f, 1.5f, 0.5f), 0.3f, gray3));
    objects.push_back(new Cube(glm::vec3(0.9f, 1.5f, 0.5f), 0.3f, gray1));

    //EYES
    Material border = {
        Color(32, 29, 36),
        1.0,
        0.0,
        9.0f,
        1.0f,
        0.0f
    };

    Material center = {
        Color(85, 133, 136),
        1.0,
        0.0,
        9.0f,
        0.5f,
        0.0f
    };

    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.9f, 0.5f, 1.25f), 0.3f, border));
    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.9f, 0.4f, 1.25f), 0.3f, border));
    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.3f, 0.5f, 1.25f), 0.3f, border));
    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.3f, 0.4f, 1.25f), 0.3f, border));
    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.85f, 0.5f, 1.35f), 0.2f, center));
    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.35f, 0.5f, 1.35f), 0.2f, center));

    //EYEBROW
    Material eyebrow1 = {
        Color(46, 46, 36),
        1.0,
        0.0,
        9.0f,
        0.0f,
        0.0f
    };

    Material eyebrow2 = {
        Color(29, 29, 27),
        1.0,
        0.0,
        9.0f,
        0.0f,
        0.0f
    };

    //right
    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.9f, 0.9f, 1.3f), 0.17f, eyebrow1));
    objects.push_back(new Cube(bigCubePosition + glm::vec3(1.05f, 0.9f, 1.3f), 0.17f, eyebrow2));
    objects.push_back(new Cube(bigCubePosition + glm::vec3(1.15f, 0.9f, 1.3f), 0.17f, eyebrow2));

    //left
    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.4f, 0.9f, 1.3f), 0.17f, eyebrow2));
    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.25f, 0.9f, 1.3f), 0.17f, eyebrow1));
    objects.push_back(new Cube(bigCubePosition + glm::vec3(0.15f, 0.9f, 1.3f), 0.17f, eyebrow1));
}

void setUpFlower() {
    //PETALS
    Material color1 = {
        Color(171, 99, 242),
        1.0,
        0.2,
        9.0f,
        0.0f,
        0.0f
    };

    Material color2 = {
        Color(180, 129, 227),
        1.0,
        0.2,
        9.0f,
        0.0f,
        0.0f
    };

    Material color3 = {
        Color(184, 118, 242),
        1.0,
        0.2,
        9.0f,
        0.0f,
        0.0f
    };

    objects.push_back(new Cube(glm::vec3(2.0f, -0.42f, 1.5f), 0.2f, color3));
    objects.push_back(new Cube(glm::vec3(2.1f, -0.32f, 1.5f), 0.2f, color1));
    objects.push_back(new Cube(glm::vec3(1.9f, -0.32f, 1.5f), 0.2f, color2));
    objects.push_back(new Cube(glm::vec3(2.0f, -0.22f, 1.5f), 0.2f, color3));

    //TALLO
    Material green1 = {
        Color(100, 158, 64),
        1.0,
        0.2,
        9.0f,
        0.0f,
        0.0f
    };

    Material green2 = {
        Color(125, 188, 89),
        1.0,
        0.2,
        9.0f,
        0.0f,
        0.0f
    };

    objects.push_back(new Cube(glm::vec3(2.0f, -0.9f, 1.5f), 0.15, green1));
    objects.push_back(new Cube(glm::vec3(2.0f, -0.8f, 1.5f), 0.15f, green2));
    objects.push_back(new Cube(glm::vec3(2.0f, -0.7f, 1.5f), 0.15f, green1));
    objects.push_back(new Cube(glm::vec3(2.0f, -0.6f, 1.5f), 0.15f, green2));

}

void render() {
    float fov = 3.1415/3;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            float screenX = (2.0f * (x + 0.5f)) / SCREEN_WIDTH - 1.0f;
            float screenY = -(2.0f * (y + 0.5f)) / SCREEN_HEIGHT + 1.0f;
            screenX *= ASPECT_RATIO;
            screenX *= tan(fov/2.0f);
            screenY *= tan(fov/2.0f);


            glm::vec3 cameraDir = glm::normalize(camera.target - camera.position);

            glm::vec3 cameraX = glm::normalize(glm::cross(cameraDir, camera.up));
            glm::vec3 cameraY = glm::normalize(glm::cross(cameraX, cameraDir));
            glm::vec3 rayDirection = glm::normalize(
                cameraDir + cameraX * screenX + cameraY * screenY
            );
           
            Color pixelColor = castRay(camera.position, rayDirection);

            point(glm::vec2(x, y), pixelColor);
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow("Hello World - FPS: 0", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                          SCREEN_WIDTH, SCREEN_HEIGHT, 
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    int frameCount = 0;
    Uint32 startTime = SDL_GetTicks();
    Uint32 currentTime = startTime;
    
    setUpBee();
    setUpFlower();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                        print("up");
                        camera.rotate(0.0f, 1.0f);
                        break;
                    case SDLK_DOWN:
                        print("down");
                        camera.rotate(0.0f, -1.0f);
                        break;
                    case SDLK_LEFT:
                        print("left");
                        camera.rotate(-1.0f, 0.0f);
                        break;
                    case SDLK_RIGHT:
                        print("right");
                        camera.rotate(1.0f, 0.0f);
                        break;
                    case SDLK_RETURN:
                        camera.move(1.0f);
                        break;
                    case SDLK_SPACE:
                        camera.move(-1.0f);
                        break;
                 }
            }


        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render();

        // Present the renderer
        SDL_RenderPresent(renderer);

        frameCount++;

        // Calculate and display FPS
        if (SDL_GetTicks() - currentTime >= 1000) {
            currentTime = SDL_GetTicks();
            std::string title = "BEEyou- FPS: " + std::to_string(frameCount);
            SDL_SetWindowTitle(window, title.c_str());
            frameCount = 0;
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

