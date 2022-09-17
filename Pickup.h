#pragma once
#include <SFML/Graphics.hpp>

using namespace sf;
class Pickup {
private:
	const int HEALTH_START_VALUE = 50;
	const int AMMO_START_VALUE = 12;
	const int START_WAIT_TIME = 10;
	const int START_SECONDS_TO_LIVE = 5;

	Sprite m_Sprite;
	IntRect m_Arena;
	int m_Value;

	//what type of pickup is it
	// 1 - health, 2 - ammo
	int m_Type;
	bool m_Spawned;
	float m_SecondsSinceSpawn, m_SecondsSinceDeSpawn, m_SecondsToLive, m_SecondsToWait;
public:
	Pickup(int type);
	void setArena(IntRect arena);
	void spawn();
	FloatRect getPosition();
	Sprite getSprite();
	void update(float elapsedTime);
	bool isSpawned();
	int gotIt();
	void upgrade();
};