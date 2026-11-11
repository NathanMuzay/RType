#pragma once

namespace engine {

class Vector2
{
public:
    Vector2(float x = 0.f, float y = 0.f);

    float x() const;
    float y() const;

private:
    float m_x;
    float m_y;
};

} // namespace engine
