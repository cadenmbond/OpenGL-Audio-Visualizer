#include "Square.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void Square::update(float deltaTime) {
    setTransparency(transparency - deltaTime);
}

void Square::draw() const {
    glColor4f(color.r, color.g, color.b, transparency);

    glBegin(GL_QUADS);
    glVertex2f(position.x, position.y);
    glVertex2f(position.x + size, position.y);
    glVertex2f(position.x + size, position.y + size);
    glVertex2f(position.x, position.y + size);
    glEnd();
}
