#include "Collision.hpp"

namespace engine::physics {

std::vector<CollisionPair> findCollisions(const std::vector<Body> &bodies)
{
    std::vector<CollisionPair> collisions;
    for (std::size_t a = 0; a < bodies.size(); ++a) {
        for (std::size_t b = a + 1; b < bodies.size(); ++b) {
            if (bodies[a].box().overlaps(bodies[b].box()))
                collisions.emplace_back(bodies[a].index(), bodies[b].index());
        }
    }
    return collisions;
}

} // namespace engine::physics
