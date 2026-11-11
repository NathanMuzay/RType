#pragma once

#include <vector>

#include "Body.hpp"
#include "CollisionPair.hpp"

namespace engine::physics {

std::vector<CollisionPair> findCollisions(const std::vector<Body> &bodies);

} // namespace engine::physics
