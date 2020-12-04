#pragma once

#include "olcConsoleGameEngine.h"

struct StaticObject
{
	/// <summary>
	/// Position vector
	/// </summary>
	float x, y;

	/// <summary>
	/// Image used to display this object
	/// </summary>
	olcSprite* sprite;
};

struct DynamicObject : StaticObject
{
	/// <summary>
	/// Velocity vector
	/// </summary>
	float vx, vy;

	bool remove;
};