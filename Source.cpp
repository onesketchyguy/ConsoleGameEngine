#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>

//#include "olcPixelGameEngine.h"
#include "olcConsoleGameEngine.h"
#include "objects.h"

using namespace std;

const double pi = 3.13159;
const wchar_t wallChar = L'#';

bool SortByFirst(const pair<float, float>& a, const pair<float, float>& b)
{
	return (a.first < b.first);
}

class olc_FPS : public olcConsoleGameEngine
{
public:
	olc_FPS() {
		m_sAppName = L"FPS game demo";
	}

	void PopulateMap()
	{
		mapWidth = mapHeight = 32;

		gameMap += L"################################";
		gameMap += L"#...............#..............#";
		gameMap += L"#.......#########.......########";
		gameMap += L"#..............##..............#";
		gameMap += L"#......##......##......##......#";
		gameMap += L"#......##..............##......#";
		gameMap += L"#..............##..............#";
		gameMap += L"###............####............#";
		gameMap += L"##.............###.............#";
		gameMap += L"#............####............###";
		gameMap += L"#..............................#";
		gameMap += L"#..............##..............#";
		gameMap += L"#..............##..............#";
		gameMap += L"#...........#####...........####";
		gameMap += L"#..............................#";
		gameMap += L"###..####....########....#######";
		gameMap += L"####.####.......######..........";
		gameMap += L"#...............#...............";
		gameMap += L"#.......#########.......##..####";
		gameMap += L"#..............##..............#";
		gameMap += L"#......##......##.......#......#";
		gameMap += L"#......##......##......##......#";
		gameMap += L"#..............##..............#";
		gameMap += L"###............####............#";
		gameMap += L"##.............###.............#";
		gameMap += L"#............####............###";
		gameMap += L"#..............................#";
		gameMap += L"#..............................#";
		gameMap += L"#..............##..............#";
		gameMap += L"#...........##..............####";
		gameMap += L"#..............##..............#";
		gameMap += L"################################";
	}

	void DrawObject(StaticObject object) {
		// Check if object visable
		float vecX = object.x - playerX;
		float vecY = object.y - playerY;

		float fDistanceFromPlayer = sqrtf(vecX * vecX + vecY * vecY);

		float eyeX = sinf(playerAngle);
		float eyeY = cosf(playerAngle);
		float objectAngle = atan2f(eyeY, eyeX) - atan2f(vecY, vecX);

		if (objectAngle < -pi) objectAngle += 2.0f * pi;
		if (objectAngle > pi) objectAngle -= 2.0 * pi;

		bool inFOV = fabs(objectAngle) < fov / 2.0f;

		if (inFOV && fDistanceFromPlayer >= 0.5f && fDistanceFromPlayer < depthOfView) {
			// Draw object
			float objectCeiling = (float)(ScreenHeight() / 2.0) - ScreenHeight() / ((float)fDistanceFromPlayer);
			float objectFloor = ScreenHeight() - objectCeiling;
			float objectHeight = objectFloor - objectCeiling;

			float objectAspectRatio = (float)object.sprite->nHeight / (float)object.sprite->nWidth;
			float objectWidth = objectHeight / objectAspectRatio;

			float middleOfObject = (0.5f * (objectAngle / (fov / 2.0f)) + 0.5f) * (float)ScreenWidth();

			for (float lx = 0; lx < objectWidth; lx++)
			{
				for (float ly = 0; ly < objectHeight; ly++)
				{
					float sampleX = lx / objectWidth;
					float sampleY = ly / objectHeight;

					wchar_t symbol = object.sprite->SampleGlyph(sampleX, sampleY);
					int objectColumn = (int)(middleOfObject + lx - (objectWidth / 2.0f));

					if (objectColumn >= 0 && objectColumn < ScreenWidth() &&
						symbol != L' ' &&
						depthBuffer[objectColumn] >= fDistanceFromPlayer) { // Only draw if object is closer than the depth for the object already in the depth buffer
						Draw(objectColumn, objectCeiling + ly, symbol, object.sprite->SampleColour(sampleX, sampleY));
						depthBuffer[objectColumn] = fDistanceFromPlayer;
					}
				}
			}
		}
	}

	float GetRotationChangeAmount(float degreesPerStep)
	{
		float changeAmount = 0;

		if (m_keys[L'Q'].bHeld)
		{
			changeAmount -= degreesPerStep;
		}
		if (m_keys[L'E'].bHeld)
		{
			changeAmount += degreesPerStep;
		}

		return changeAmount;
	}

	float GetHorizontalInputChangeAmount(float degreesPerStep)
	{
		float changeAmount = 0;

		if (m_keys[L'A'].bHeld)
		{
			changeAmount -= degreesPerStep;
		}
		if (m_keys[L'D'].bHeld)
		{
			changeAmount += degreesPerStep;
		}

		return changeAmount;
	}

	float GetForwardInputChangeAmount(float degreesPerStep)
	{
		float changeAmount = 0;

		if (m_keys[L'S'].bHeld)
		{
			changeAmount -= degreesPerStep;
		}
		if (m_keys[L'W'].bHeld)
		{
			changeAmount += degreesPerStep;
		}

		return changeAmount;
	}

	void HandleInput(float elapsedTimeSinceLastFrame)
	{
		float moveSpeed = 2;
		float rotSpeed = 0.8f;

		float runMultiplier = 2;

		if (m_keys[L' '].bHeld)
		{
			rotSpeed *= runMultiplier;
			moveSpeed *= runMultiplier;
		}

		// Controls
		// Handle rotation
		playerAngle += GetRotationChangeAmount(rotSpeed * elapsedTimeSinceLastFrame);

		float sin = sinf(playerAngle);
		float cos = cosf(playerAngle);

		float fInput = GetForwardInputChangeAmount(moveSpeed);
		float hInput = GetHorizontalInputChangeAmount(moveSpeed);

		// Move player
		// Forward
		float newPlayerX = sin * fInput * elapsedTimeSinceLastFrame;
		float newPlayerY = cos * fInput * elapsedTimeSinceLastFrame;
		//Strafe
		newPlayerX += cos * hInput * elapsedTimeSinceLastFrame;
		newPlayerY += sin * -hInput * elapsedTimeSinceLastFrame;

		// Collision GetHorizontalInputChangeAmount(moveSpeed)heck
		if (gameMap[((int)(playerY + newPlayerY) * mapWidth + (int)(playerX + newPlayerX))] != '#')
		{
			playerX += newPlayerX;
			playerY += newPlayerY;
		}

		// Fire projectiles
		if (m_keys[L'F'].bReleased) {
			// Fire a projectile

			// Create the object
			DynamicObject object;
			object.x = playerX;
			object.y = playerY;

			// Add velocity to the object
			float noise = (((float)rand() / (float)RAND_MAX) - 0.5f) * 0.1f;
			object.vx = sinf(playerAngle + noise) * 8.0f;
			object.vy = cosf(playerAngle + noise) * 8.0f;

			object.sprite = spriteFireBall;
			object.remove = false;

			// Add the object to the dynamic objects list
			dynamicObjects.push_back(object);
		}
	}

	virtual bool OnUserCreate() {
		PopulateMap();

		spriteWall = new olcSprite(L"C:/Users/frres/source/repos/ConsoleGameEngine/Images/wall.spr");
		spriteLamp = new olcSprite(L"C:/Users/frres/source/repos/ConsoleGameEngine/Images/lamp.spr");
		spriteFireBall = new olcSprite(L"C:/Users/frres/source/repos/ConsoleGameEngine/Images/fireBall.spr");

		depthBuffer = new float[ScreenWidth()];

		staticObjects = {
			{8.5f, 8.5f, spriteLamp},
			{7.5f, 7.5f, spriteLamp},
			{10.5f, 3.5f, spriteLamp}
		};

		return true;
	}

	virtual bool OnUserUpdate(float elapsedTime) {
		if (m_keys[L'M'].bReleased)
		{
			drawMap = !drawMap;
		}

		int screenWidth = ScreenWidth();
		int screenHeight = ScreenHeight();

		HandleInput((float)elapsedTime);

		// Draw screen
		for (int x = 0; x < screenWidth; x++)
		{
			// Foreach column, calculate the projected ra angle into world space
			float rayAngle = (playerAngle - fov / 2.0f) + ((float)x / (float)screenWidth) * fov;
			float distanceToWall = 0;

			bool hitWall = false;
			bool boundary = false;

			// Unit vector for ray in player space
			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);

			float sampleX = 0;

			while (hitWall == false && distanceToWall < depthOfView)
			{
				distanceToWall += 0.01f;

				int testX = (int)(playerX + eyeX * distanceToWall);
				int testY = (int)(playerY + eyeY * distanceToWall);

				// Test if ray is out of bounds
				if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight)
				{
					// Set distance to maximum depth
					hitWall = true;
					distanceToWall = depthOfView;
				}
				else
				{
					if (gameMap[testY * mapWidth + testX] == wallChar)
					{
						hitWall = true;

						// Determine where the ray hit the wall.

						// Get the middle of the cell
						float blockMidX = (float)testX + 0.5f;
						float blockMidY = (float)testY + 0.5f;

						// Get the point where the collision has occured
						float testPointX = playerX + eyeX * distanceToWall;
						float testPointY = playerY + eyeY * distanceToWall;

						// Get the angle of the hit
						float testAngle = atan2f(testPointY - blockMidY, testPointX - blockMidX);

						// Get the sample point from the texture on the wall
						if (testAngle >= -pi * 0.25f && testAngle < pi * 0.25f)
							sampleX = testPointY - (float)testY;
						if (testAngle >= pi * 0.25f && testAngle < pi * 0.75f)
							sampleX = testPointX - (float)testX;
						if (testAngle < -pi * 0.25f && testAngle >= -pi * 0.75f)
							sampleX = testPointX - (float)testX;
						if (testAngle >= pi * 0.75f || testAngle < -pi * 0.75f)
							sampleX = testPointY - (float)testY;
					}
				}
			}

			// Calculate the distance to ceiling and floor
			int ceiling = (float)(screenHeight / 2.0) - screenHeight / ((float)distanceToWall);
			int floor = screenHeight - ceiling;

			// Update the depth buffer
			depthBuffer[x] = distanceToWall;

			for (int y = 0; y < screenHeight; y++)
			{
				if (y <= ceiling)
				{
					// Ceiling
					Draw(x, y, PIXEL_SOLID, FG_BLACK);
				}
				else
				{
					if (y > ceiling && y <= floor)
					{
						// Wall
						if (distanceToWall < depthOfView) {
							float sampleY = ((float)y - (float)ceiling) / ((float)floor - (float)ceiling);
							Draw(x, y, spriteWall->SampleGlyph(sampleX, sampleY), spriteWall->SampleColour(sampleX, sampleY));
						}
						else
						{
							Draw(x, y, PIXEL_SOLID, FG_BLACK);
						}
					}
					else
					{
						// Floor
						Draw(x, y, PIXEL_SOLID, BG_DARK_GREEN);
					}
				}
			}
		}

		// Draw all static objects
		for (auto& object : staticObjects) DrawObject(object);

		// Update the dynamic objects
		for (auto& object : dynamicObjects)
		{
			// add the velocity of this object
			object.x += object.vx * elapsedTime;
			object.y += object.vy * elapsedTime;

			// check for collision
			if (gameMap.c_str()[(int)object.x * mapWidth + (int)object.y] == wallChar)
				object.remove = true;

			DrawObject(object);
		}

		// Run a lambda function to check each object if they should be removed or not.
		// Then remove those objects if we should.
		dynamicObjects.remove_if([](DynamicObject& obj) {return obj.remove; });

		if (drawMap)
		{
			// Display Map
			for (int nx = 0; nx < mapWidth; nx++) for (int ny = 0; ny < mapWidth; ny++)
				Draw(nx + 1, ny, gameMap[ny * mapWidth + nx]);

			Draw((int)playerX + 1, (int)playerY, L'o');
		}

		return true;
	}

private:
	float playerX = 8;
	float playerY = 8;
	float playerAngle = 0.0f;

	float fov = pi / 4.5;
	float depthOfView = 10;

	int mapWidth;
	int mapHeight;

	wstring gameMap;

	bool drawMap = false;

	olcSprite* spriteWall;
	olcSprite* spriteLamp;
	olcSprite* spriteFireBall;

	float* depthBuffer = nullptr;

	list<StaticObject> staticObjects;
	list<DynamicObject> dynamicObjects;
};

int main()
{
	olc_FPS game;
	game.ConstructConsole(240, 160, 4, 4);
	game.Start();

	return 0;
}