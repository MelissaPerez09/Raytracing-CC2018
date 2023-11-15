#include "cube.h"

Cube::Cube(const glm::vec3& position, float sideLength, const Material& mat)
  : position(position), sideLength(sideLength), Object(mat) {}

Intersect Cube::rayIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const {
  // Calculate half the side length for convenience
  float halfSideLength = sideLength / 2.0f;

  // Calculate the minimum and maximum bounds of the cube along each axis
  glm::vec3 minBounds = position - glm::vec3(halfSideLength);
  glm::vec3 maxBounds = position + glm::vec3(halfSideLength);

  // Compute the intersection of the ray with the axis-aligned bounding box (AABB) of the cube
  float tmin = (minBounds.x - rayOrigin.x) / rayDirection.x;
  float tmax = (maxBounds.x - rayOrigin.x) / rayDirection.x;

  if (tmin > tmax) {
    std::swap(tmin, tmax);
  }

  float tymin = (minBounds.y - rayOrigin.y) / rayDirection.y;
  float tymax = (maxBounds.y - rayOrigin.y) / rayDirection.y;

  if (tymin > tymax) {
    std::swap(tymin, tymax);
  }

  if ((tmin > tymax) || (tymin > tmax)) {
    return Intersect{false};
  }

  if (tymin > tmin) {
    tmin = tymin;
  }

  if (tymax < tmax) {
    tmax = tymax;
  }

  float tzmin = (minBounds.z - rayOrigin.z) / rayDirection.z;
  float tzmax = (maxBounds.z - rayOrigin.z) / rayDirection.z;

  if (tzmin > tzmax) {
    std::swap(tzmin, tzmax);
  }

  if ((tmin > tzmax) || (tzmin > tmax)) {
    return Intersect{false};
  }

  if (tzmin > tmin) {
    tmin = tzmin;
  }

  if (tzmax < tmax) {
    tmax = tzmax;
  }

  // The ray intersects the AABB of the cube; calculate the intersection point and normal
  glm::vec3 point = rayOrigin + tmin * rayDirection;
  glm::vec3 normal;

  // Determine which face of the cube the intersection occurred on and set the normal accordingly
  if (tmin == tmax) {
    // Ray intersects one of the faces along the x-axis
    normal = glm::vec3((point.x < position.x) ? -1.0f : 1.0f, 0.0f, 0.0f);
  } else if (tymin == tymax) {
    // Ray intersects one of the faces along the y-axis
    normal = glm::vec3(0.0f, (point.y < position.y) ? -1.0f : 1.0f, 0.0f);
  } else {
    // Ray intersects one of the faces along the z-axis
    normal = glm::vec3(0.0f, 0.0f, (point.z < position.z) ? -1.0f : 1.0f);
  }

  return Intersect{true, tmin, point, glm::normalize(normal)};
}
