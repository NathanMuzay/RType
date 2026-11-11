#pragma once

namespace engine {

class IntRect
{
public:
    IntRect(int x = 0, int y = 0, int width = 0, int height = 0);

    int x() const;
    int y() const;
    int width() const;
    int height() const;

private:
    int m_x;
    int m_y;
    int m_width;
    int m_height;
};

} // namespace engine
