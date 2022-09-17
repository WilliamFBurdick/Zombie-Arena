#pragma once
#include <SFML/Graphics.hpp>
using namespace sf;
class Player {
private:
	const float START_SPEED = 200;
	const float START_HEALTH = 100;
	Vector2f m_Position;
	Sprite m_Sprite;
	Texture m_Texture;
	Vector2f m_Resolution;
	IntRect m_Arena;
	int m_TileSize;

	//Inputs
	bool m_UpPressed, m_DownPressed, m_LeftPressed, m_RightPressed;
	int m_Health, m_MaxHealth;
	Time m_LastHit;
	float m_Speed;
public:
	Player();
	void spawn(IntRect arena, Vector2f resolution, int tileSize);
	void resetPlayerStats();

	bool hit(Time timeHit);
	Time getLastHitTime();
	FloatRect getPosition();
	Vector2f getCenter();
	float getRotation();
	Sprite getSprite();
	//movement methods
	void moveLeft();
	void moveRight();
	void moveUp();
	void moveDown();
	//stop movement methods
	void stopLeft();
	void stopRight();
	void stopUp();
	void stopDown();

	void update(float elapsedTime, Vector2i mousePosition);
	//upgrade methods
	void upgradeSpeed();
	void upgradeHealth();
	void increaseHealthLevel(int amount);
	int getHealth();
};