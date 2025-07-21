#pragma once
#ifndef SQUARE_H
#define SQUARE_H

#include <glm/glm.hpp>

class Square {
public:
	glm::vec2 position;
	glm::vec3 color;

	float size = 10.0f;
	float transparency = 1.0f;

	void setTransparency(float newTransparency) { transparency = newTransparency; }
	void update(float deltaTime);
	void draw() const;
};

#endif