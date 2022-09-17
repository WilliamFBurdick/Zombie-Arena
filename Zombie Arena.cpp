#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>
#include <fstream>
#include "Player.h"
#include "ZombieArena.h"
#include "TextureHolder.h"
#include "Bullet.h"
#include "Pickup.h"

#include <iostream>

using namespace sf;
int main()
{
	TextureHolder holder;

	enum class State { PAUSED, LEVELING_UP, GAME_OVER, PLAYING };

	State state = State::GAME_OVER;
	//Set up game window
	Vector2f resolution;
	resolution.x = 1920;
	resolution.y = 1080;
	RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena", Style::Default);
	//Initialize main game view
	View mainView(sf::FloatRect(0, 0, resolution.x, resolution.y));
	//Time variables
	Clock clock;
	Time gameTimeTotal;
	Vector2f mouseWorldPosition;
	Vector2i mouseScreenPosition;
	//Set up game actors
	Player player;
	//Set up the arena
	IntRect arena;
	//Create the background
	VertexArray background;
	Texture textureBackground = TextureHolder::GetTexture("graphics/background_sheet.png");
	//Zombie horde variables
	int numZombies;
	int numZombiesAlive;
	Zombie* zombies = nullptr;
	//Bullet handling variables
	Bullet bullets[100];
	int currentBullet = 0;
	int bulletsSpare = 24;
	int bulletsInClip = 6;
	int clipSize = 6;
	float fireRate = 1;
	//Last time fire button pressed
	Time lastPressed;
	//Hide mouse pointer and replace it with crosshair
	window.setMouseCursorVisible(false);
	Sprite spriteCrosshair;
	Texture textureCrosshair = TextureHolder::GetTexture("graphics/crosshair.png");
	spriteCrosshair.setTexture(textureCrosshair);
	spriteCrosshair.setOrigin(25, 25);
	//Create some pickups
	Pickup healthPickup(1);
	Pickup ammoPickup(2);
	//Game mode variables (score)
	int score = 0;
	int hiScore = 0;
	//HUD variables
	//home/game over screen
	Sprite spriteGameOver;
	Texture textureGameOver = TextureHolder::GetTexture("graphics/background.png");
	spriteGameOver.setTexture(textureGameOver);
	spriteGameOver.setPosition(0, 0);
	//create a HUD view
	View hudView(sf::FloatRect(0, 0, resolution.x, resolution.y));
	//create ammo icon sprite
	Sprite spriteAmmoIcon;
	Texture textureAmmoIcon = TextureHolder::GetTexture("graphics/ammo_icon.png");
	spriteAmmoIcon.setTexture(textureAmmoIcon);
	spriteAmmoIcon.setPosition(20, 980);
	//Font
	Font font;
	font.loadFromFile("fonts/zombiecontrol.ttf");
	//pause screen
	Text pausedText;
	pausedText.setFont(font);
	pausedText.setCharacterSize(155);
	pausedText.setFillColor(Color::White);
	pausedText.setPosition(400, 400);
	pausedText.setString("Presss Enter \nto continue");
	//game over screen
	Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setCharacterSize(125);
	gameOverText.setFillColor(Color::White);
	gameOverText.setPosition(250, 850);
	gameOverText.setString("Press Enter to play");
	//level up screen
	Text levelUpText;
	levelUpText.setFont(font);
	levelUpText.setCharacterSize(80);
	levelUpText.setFillColor(Color::White);
	levelUpText.setPosition(150, 250);
	std::stringstream levelUpStream;
	levelUpStream <<
		"1 - Increased rate of fire\n" <<
		"2 - Increased clip size\n" <<
		"3 - Increased max health\n" <<
		"4 - Increased run speed\n" <<
		"5 - More and better health pickups\n" <<
		"6 - More and better ammo pickups";
	levelUpText.setString(levelUpStream.str());
	//player ammo
	Text ammoText;
	ammoText.setFont(font);
	ammoText.setCharacterSize(55);
	ammoText.setFillColor(Color::White);
	ammoText.setPosition(200, 980);
	//score
	Text scoreText;
	scoreText.setFont(font);
	scoreText.setCharacterSize(55);
	scoreText.setFillColor(Color::White);
	scoreText.setPosition(20, 0);
	//score file
	std::ifstream inputFile("gamedata/scores.txt");
	if (inputFile.is_open())
	{
		inputFile >> hiScore;
		inputFile.close();
	}
	//high score
	Text hiScoreText;
	hiScoreText.setFont(font);
	hiScoreText.setCharacterSize(55);
	hiScoreText.setFillColor(Color::White);
	hiScoreText.setPosition(1400, 0);
	std::stringstream s;
	s << "Hi Score: " << hiScore;
	hiScoreText.setString(s.str());
	//remaining zombies
	Text zombiesRemainingText;
	zombiesRemainingText.setFont(font);
	zombiesRemainingText.setCharacterSize(55);
	zombiesRemainingText.setFillColor(Color::White);
	zombiesRemainingText.setPosition(1500, 980);
	zombiesRemainingText.setString("Zombies: 100");
	//wave number
	int wave = 0;
	Text waveNumberText;
	waveNumberText.setFont(font);
	waveNumberText.setCharacterSize(55);
	waveNumberText.setFillColor(Color::White);
	waveNumberText.setPosition(1250, 980);
	waveNumberText.setString("Wave: 0");
	//health bar
	RectangleShape healthBar;
	healthBar.setFillColor(Color::Red);
	healthBar.setPosition(450, 980);

	//HUD update variables
	int framesSinceLastHUDUpdate = 0;
	int fpsMeasurementFrameInterval = 1000;

	//Sounds
	//hit sound
	SoundBuffer hitBuffer;
	hitBuffer.loadFromFile("sound/hit.wav");
	Sound hit;
	hit.setBuffer(hitBuffer);
	//splat sound
	SoundBuffer splatBuffer;
	splatBuffer.loadFromFile("sound/splat.wav");
	Sound splat;
	splat.setBuffer(splatBuffer);
	//shoot sound
	SoundBuffer shootBuffer;
	shootBuffer.loadFromFile("sound/shoot.wav");
	Sound shoot;
	shoot.setBuffer(shootBuffer);
	//reload sound
	SoundBuffer reloadBuffer;
	reloadBuffer.loadFromFile("sound/reload.wav");
	Sound reload;
	reload.setBuffer(reloadBuffer);
	//reload failed sound
	SoundBuffer reloadFailedBuffer;
	reloadFailedBuffer.loadFromFile("sound/reload_failed.wav");
	Sound reloadFailed;
	reloadFailed.setBuffer(reloadFailedBuffer);
	//powerup sound
	SoundBuffer powerupBuffer;
	powerupBuffer.loadFromFile("sound/powerup.wav");
	Sound powerup;
	powerup.setBuffer(powerupBuffer);
	//pickup sound
	SoundBuffer pickupBuffer;
	pickupBuffer.loadFromFile("sound/pickup.wav");
	Sound pickup;
	pickup.setBuffer(pickupBuffer);

	//Main game loop
	while (window.isOpen())
	{
		/************************/
		/* Handle player input */
		/************************/
		// Handle events
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::KeyPressed)
			{
				if (event.key.code == Keyboard::Return && state == State::PLAYING) state = State::PAUSED;
			}
			else if (event.key.code == Keyboard::Return && state == State::PAUSED)
			{
				state = State::PLAYING;
				
				//restart clock to prevent frame jump
				clock.restart();
			}
			else if (event.key.code == Keyboard::Return && state == State::GAME_OVER)
			{
				state = State::LEVELING_UP;
				wave = 0;
				score = 0;
				currentBullet = 0;
				bulletsSpare = 24;
				bulletsInClip = 6;
				clipSize = 6;
				fireRate = 1;
				player.resetPlayerStats();
			}

			if (state == State::PLAYING)
			{
				//handle reload key press
				if (event.key.code == Keyboard::R)
				{
					int ammoNeeded = clipSize - bulletsInClip;
					if (bulletsSpare >= ammoNeeded)
					{
						bulletsInClip = clipSize;
						bulletsSpare -= ammoNeeded;
						reload.play();
					}
					else if (bulletsSpare > 0)
					{
						bulletsInClip += bulletsSpare;
						bulletsSpare = 0;
						reload.play();
					}
					else
					{
						reloadFailed.play();
					}
				}
			}
		}
		//Handle quitting
		if (Keyboard::isKeyPressed(Keyboard::Escape)) window.close();
		//Handle inputs related to gameplay
		if (state == State::PLAYING)
		{
			//Handle movement input
			if (Keyboard::isKeyPressed(Keyboard::W)) player.moveUp();
			else player.stopUp();
			if (Keyboard::isKeyPressed(Keyboard::S)) player.moveDown();
			else player.stopDown();
			if (Keyboard::isKeyPressed(Keyboard::A)) player.moveLeft();
			else player.stopLeft();
			if (Keyboard::isKeyPressed(Keyboard::D)) player.moveRight();
			else player.stopRight();

			//Handle shooting input
			if (Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if (gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate && bulletsInClip > 0)
				{
					bullets[currentBullet].shoot(player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y);
					currentBullet++;
					if (currentBullet > 99) currentBullet = 0;
					lastPressed = gameTimeTotal;
					shoot.play();
					bulletsInClip--;
				}
			}
		}
		//Handle leveling up state input
		if (state == State::LEVELING_UP)
		{
			if (event.key.code == Keyboard::Num1)
			{
				//upgrade fire rate
				fireRate++;
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num2)
			{
				//upgrade clip size
				clipSize += clipSize;
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num3)
			{
				//upgrade health
				player.upgradeHealth();
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num4)
			{
				//upgrade speed
				player.upgradeSpeed();
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num5)
			{
				//upgrade health pickup
				healthPickup.upgrade();
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num6)
			{
				//upgrade ammo pickup
				ammoPickup.upgrade();
				state = State::PLAYING;
			}

			if (state == State::PLAYING)
			{
				wave++;
				//Initialize the game for a new game
				arena.width = 500 * wave;
				arena.height = 500 * wave;
				arena.left = 0;
				arena.top = 0;
				int tileSize = createBackground(background, arena);
				player.spawn(arena, resolution, tileSize);
				//configure the pickups
				healthPickup.setArena(arena);
				ammoPickup.setArena(arena);
				//create the zombie horde
				numZombies = 5 * wave;
				//delete the existing horde (if any)
				delete[] zombies;
				zombies = createHorde(numZombies, arena);
				numZombiesAlive = numZombies;
				powerup.play();
				//restart clock to prevent frame jump
				clock.restart();
			}
		}
		/***************************/
		/* Update the game objects*/
		/***************************/
		if (state == State::PLAYING)
		{
			//update delta time and the game clock
			Time dt = clock.restart();
			gameTimeTotal += dt;
			float dtAsSeconds = dt.asSeconds();
			//get the mouse screen position and convert it to a world position
			mouseScreenPosition = Mouse::getPosition();
			mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(), mainView);
			//update the crosshair
			spriteCrosshair.setPosition(mouseWorldPosition);
			//update the player game object
			player.update(dtAsSeconds, Mouse::getPosition());
			Vector2f playerPosition(player.getCenter());

			mainView.setCenter(player.getCenter());

			//update the zombies
			for (int i = 0; i < numZombies; i++)
			{
				if (zombies[i].isAlive()) zombies[i].update(dt.asSeconds(), playerPosition);
			}
			//update the bullets
			for (int i = 0; i < 100; i++)
			{
				if (bullets[i].isInFlight()) bullets[i].update(dtAsSeconds);
			}
			//update the pickups
			healthPickup.update(dtAsSeconds);
			ammoPickup.update(dtAsSeconds);
			//Collision detection
			//check if zombies have been shot
			for (int i = 0; i < 100; i++)
			{
				for (int j = 0; j < numZombies; j++)
				{
					if (bullets[i].isInFlight() && zombies[j].isAlive())
					{
						if (bullets[i].getPosition().intersects(zombies[j].getPosition()))
						{
							//stop the bullet
							bullets[i].stop();
							//hit the zombie and check if it's killed
							if (zombies[j].hit())
							{
								//zombie is killed
								score += 10;
								if (score >= hiScore) hiScore = score;
								numZombiesAlive--;
								//when all zombies are dead
								if (numZombiesAlive == 0) state = State::LEVELING_UP;
							}
							splat.play();
						}
					}
				}
			}
			//check if player has been hit by zombie
			for (int i = 0; i < numZombies; i++)
			{
				if (player.getPosition().intersects(zombies[i].getPosition()) && zombies[i].isAlive())
				{
					if (player.hit(gameTimeTotal))
					{
						hit.play();
					}
					if (player.getHealth() <= 0)
					{
						state = State::GAME_OVER;
						std::ofstream outputFile("gamedata/scores.txt");
						outputFile << hiScore;
						outputFile.close();
					}
				}
			}
			//check if player touched health pickup
			if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned())
			{
				player.increaseHealthLevel(healthPickup.gotIt());
				pickup.play();
			}
			//check if player touched ammo pickup
			if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned())
			{
				bulletsSpare += ammoPickup.gotIt();
				reload.play();
			}

			//size the health bar
			healthBar.setSize(Vector2f(player.getHealth() * 3, 50));
			//increment number of frames since last update
			framesSinceLastHUDUpdate++;
			//check if time for HUD update
			if (framesSinceLastHUDUpdate > fpsMeasurementFrameInterval)
			{
				//update the HUD text
				std::stringstream ssAmmo, ssScore, ssHiScore, ssWave, ssZombiesAlive;
				//ammo text
				ssAmmo << bulletsInClip << "/" << bulletsSpare;
				ammoText.setString(ssAmmo.str());
				//score text
				ssScore << "Score: " << score;
				scoreText.setString(ssScore.str());
				ssHiScore << "Hi Score: " << hiScore;
				hiScoreText.setString(ssHiScore.str());
				//update wave
				ssWave << "Wave: " << wave;
				waveNumberText.setString(ssWave.str());
				//update zombie count
				ssZombiesAlive << "Zombies: " << numZombiesAlive;
				zombiesRemainingText.setString(ssZombiesAlive.str());
				framesSinceLastHUDUpdate = 0;
			}
		}
		/********************/
		/* Render the game */
		/********************/
		if (state == State::PLAYING)
		{
			//clear the screen
			window.clear();
			//set the main view to be displayed and drawn to
			window.setView(mainView);
			//draw the background
			window.draw(background, &textureBackground);
			//draw the zombies
			for (int i = 0; i < numZombies; i++) window.draw(zombies[i].getSprite());
			//draw the bullets
			for (int i = 0; i < 100; i++)
			{
				if (bullets[i].isInFlight()) window.draw(bullets[i].getShape());
			}
			//draw the player
			window.draw(player.getSprite());
			//draw pickups (if spawned)
			if (ammoPickup.isSpawned()) window.draw(ammoPickup.getSprite());
			if (healthPickup.isSpawned()) window.draw(healthPickup.getSprite());

			//draw the crosshair
			window.draw(spriteCrosshair);
			//switch to HUD view
			window.setView(hudView);
			//draw the HUD elements
			window.draw(spriteAmmoIcon);
			window.draw(ammoText);
			window.draw(scoreText);
			window.draw(hiScoreText);
			window.draw(healthBar);
			window.draw(waveNumberText);
			window.draw(zombiesRemainingText);
		}
		if (state == State::LEVELING_UP)
		{
			window.draw(spriteGameOver);
			window.draw(levelUpText);
		}
		if (state == State::PAUSED)
		{
			window.draw(pausedText);
		}
		if (state == State::GAME_OVER)
		{
			window.draw(spriteGameOver);
			window.draw(gameOverText);
			window.draw(scoreText);
			window.draw(hiScoreText);
		}

		window.display();
	}
	//free the allocated memory
	delete[] zombies;
	return 0;
}