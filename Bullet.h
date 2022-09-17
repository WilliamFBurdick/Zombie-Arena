#pragma once
#include <SFML/Graphics.hpp>
using namespace sf;

class Bullet {
private:
	Vector2f m_Position;
	RectangleShape m_BulletShape;
	bool m_InFlight = false;
	float m_BulletSpeed = 1000;
	float m_BulletDistanceX;
	float m_BulletDistanceY;

	float m_MaxX, m_MinX, m_MaxY, m_MinY;
public:
	Bullet();
	void stop();
	bool isInFlight();
	void shoot(float startX, float startY, float xTarget, float yTarget);
	FloatRect getPosition();
	RectangleShape getShape();
	void update(float elapsedTime);
};