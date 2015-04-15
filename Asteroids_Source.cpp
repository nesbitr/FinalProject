#include <Simple2D.h>
#include "SimpleAsteroids.h"

// Update game world object position
void GameWorldObject::Update()
{
	// Re-activate if the inactive timer has expired
	if (!active)
	{
		if (GetTickCount() < reactivateTime)
			return;

		active = true;

		// Set up object position, speed etc.
		Reset();
	}

	// Update position based on movement speed (velocity vector)
	x += vX;
	y += vY;

	// Use Stokes' law to apply drag to the object
	vX = vX - vX * dragFactor;
	vY = vY - vY * dragFactor;

	// Update rotation based on rotational speed
	angle += rotationalSpeed;

	// Keep object in game world (wrap-around borders)
	if (x < 0) x += renderer->ResolutionX;
	if (x >= renderer->ResolutionX) x -= renderer->ResolutionX;
	if (y < 0) y += renderer->ResolutionY;
	if (y >= renderer->ResolutionY) y -= renderer->ResolutionY;
}

// Draw a game world object
void GameWorldObject::Draw()
{
	if (!active) return;

	Matrix loc = geometry.Rotate(angle, Geometry::PointCenter) * geometry.Move(static_cast<int>(x), static_cast<int>(y));
		
	// Draw the object in its main position
	geometry.Draw(loc, Geometry::Center, outlineBrush);

	// Check to see if any part of the bounding box is off the edge of the screen, in which case we want to render a wrapped copy too
	D2D1_RECT_F bounds = geometry.GetBounds(loc);

	// If the object is partially beyond one edge...
	if (bounds.left < 0)
		geometry.Draw(loc * geometry.Move(renderer->ResolutionX, 0), Geometry::Center, outlineBrush);

	if (bounds.right >= renderer->ResolutionX)
		geometry.Draw(loc * geometry.Move(-renderer->ResolutionX, 0), Geometry::Center, outlineBrush);

	if (bounds.top < 0)
		geometry.Draw(loc * geometry.Move(0, renderer->ResolutionY), Geometry::Center, outlineBrush);

	if (bounds.bottom >= renderer->ResolutionY)
		geometry.Draw(loc * geometry.Move(0, -renderer->ResolutionY), Geometry::Center, outlineBrush);

	// If the object is partially beyond two edges (corners)...
	if (bounds.left < 0 && bounds.top < 0)													// Top-left corner (redraw at bottom-right)
		geometry.Draw(loc * geometry.Move(renderer->ResolutionX, renderer->ResolutionY), Geometry::Center, outlineBrush);

	if (bounds.right >= renderer->ResolutionX && bounds.top < 0)							// Top-right corner (redraw at bottom-left)
		geometry.Draw(loc * geometry.Move(-renderer->ResolutionX, renderer->ResolutionY), Geometry::Center, outlineBrush);

	if (bounds.left < 0 && bounds.bottom >= renderer->ResolutionY)							// Bottom-left corner (redraw at top-right)
		geometry.Draw(loc * geometry.Move(renderer->ResolutionX, -renderer->ResolutionY), Geometry::Center, outlineBrush);

	if (bounds.right >= renderer->ResolutionX && bounds.bottom >= renderer->ResolutionY)	// Bottom-right corner (redraw at top-left)
		geometry.Draw(loc * geometry.Move(-renderer->ResolutionX, -renderer->ResolutionY), Geometry::Center, outlineBrush);
}

// Collision detection
bool GameWorldObject::IsCollision(GameWorldObject &o)
{
	if (!active) return false;

	Matrix loc1 = geometry.Rotate(angle, Geometry::PointCenter) * geometry.Move(static_cast<int>(x), static_cast<int>(y));
	Matrix loc2 = o.geometry.Rotate(o.angle, Geometry::PointCenter) * o.geometry.Move(static_cast<int>(o.x), static_cast<int>(o.y));

	return renderer->GeometryCollision(geometry, o.geometry, loc1, loc2);
}

// Make inactive for X ms
void GameWorldObject::MakeInactiveFor(int x)
{
	active = false;
	reactivateTime = GetTickCount() + x;
}

// One-time player setup
Player::Player(Simple2D *r) : GameWorldObject(r, r->MakeBrush(Colour::White), 0.02f, 0.0f), rotFactor(5.0f), accFactor(0.2f)
{
	// Create ship geometry
	int xTop = 0;
	int xSize = 25;
	int ySize = 30;
	int yTop = -ySize / 2;
	int yIndent = 10;

	GeometryData gd = renderer->StartCreatePath(xTop, yTop);
	
#pragma warning ( disable : 4244 )
	D2D1_POINT_2F sp[] = {
		{ xTop + xSize / 2,		yTop + ySize },
		{ xTop,					yTop + ySize - yIndent },
		{ xTop - xSize / 2,		yTop + ySize }
	};
#pragma warning ( default : 4244 )

	gd->AddLines(sp, 3);

	geometry = renderer->EndCreatePath();

	// Number of bullets allowed on-screen from this ship at once
	maxBullets = 12;

	// Fire cooldown time (ms)
	bulletCooldown = 200;

	// Three lives remaining
	Lives = 3;

	// No score yet
	Score = 0;

	// Shield parameters
	shieldDuration = 4000;
	shieldCooldown = 15000;
}

// Per-life ship setup
void Player::Reset()
{
	// Ship position
	x = static_cast<float>(renderer->ResolutionX) / 2;
	y = static_cast<float>(renderer->ResolutionY) / 2;

	// Ship orientation
	angle = 0;

	// Velocity
	vX = vY = 0;

	// No bullets fired yet
	lastBulletTime = 0;
	bulletsUsed = 0;

	// Lose a life
	Lives--;

	// Make invincible
	invincibility = true;
	lastShieldTime = GetTickCount();
}

// Rotate the ship
void Player::Rotate(int direction)
{
	angle += direction * rotFactor;
}

// Accelerate the ship in the current direction
void Player::Accelerate()
{
	// Create a normalized vector in the direction of travel
	float xN = static_cast<float>(sin(2 * M_PI * (angle / 360)));
	float yN = static_cast<float>(cos(2 * M_PI * (angle / 360)));

	// Add to velocity vector (using minus for y because Direct2D uses 0,0 as the top-left corner instead of bottom-left)
	vX += xN * accFactor;
	vY -= yN * accFactor;
}

// Fire a bullet
Bullet *Player::Fire()
{
	// Don't fire unless the cooldown period has expired
	if (GetTickCount() - lastBulletTime >= bulletCooldown)
	{
		// Don't fire if the maximum number of bullets are already on screen
		if (bulletsUsed < maxBullets)
		{
			// Make new bullet
			Bullet *bullet = new Bullet(renderer, *this, x, y, angle);

			// Last bullet fired now
			lastBulletTime = GetTickCount();
			bulletsUsed++;

			return bullet;
		}
	}
	return NULL;
}

// Update player status
void Player::Update()
{
	// Update shield flag
	invincibility = (GetTickCount() - lastShieldTime) < shieldDuration;

	// Do normal update
	GameWorldObject::Update();
}

// Draw ancillary stuff relating to the player
void Player::Draw()
{
	// Draw ship if active
	GameWorldObject::Draw();

	// Draw a circle around the ship if currently invincible (shields)
	if (active && invincibility)
		renderer->DrawRoundedRectangleWH(static_cast<int>(x) - 30, static_cast<int>(y) - 30, 60, 60, 30, 30, Colour::Red);

	// Draw lives
	renderer->Text(20, 12, "Lives", L"Verdana", 18.0f, Colour::White);

	for (int i = 0; i < Lives; i++)
		renderer->FillRoundedRectangleWH(i * 20 + 20, 42, 10, 10, 5, 5, Colour::Green);

	// Draw score
	renderer->Text(0, 12, "Score", L"Verdana", 18.0f, Colour::White, DWRITE_TEXT_ALIGNMENT_TRAILING,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, renderer->ResolutionX - 20);

	renderer->Text(0, 32, StringFactory(Score), L"Verdana", 24.0f, Colour::CornflowerBlue, DWRITE_TEXT_ALIGNMENT_TRAILING,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, renderer->ResolutionX - 18);

	// Indicate if shield available
	renderer->Text(20, renderer->ResolutionY - 32, "Shield", L"Verdana", 18.0f, Colour::White);

	renderer->FillRectangleWH(85, renderer->ResolutionY - 29, 16, 16,
		(GetTickCount() - lastShieldTime > shieldCooldown && !invincibility)? Colour::Green : Colour::Red);

}

// Stop firing a bullet (called back when a Bullet object is destroyed)
void Player::EndFire()
{
	bulletsUsed = max(bulletsUsed - 1, 0);
}

// Reset cooldown (used when fire key is released)
void Player::ResetBulletCooldown()
{
	lastBulletTime = 0;
}

// Try to activate shield
void Player::ActivateShield()
{
	if (GetTickCount() - lastShieldTime > shieldCooldown && !invincibility)
	{
		invincibility = true;
		lastShieldTime = GetTickCount();
	}
}

// Create a new asteroid
Asteroid::Asteroid(Simple2D *r, int size, float px, float py, float vx, float vy)
	: GameWorldObject(r, r->MakeBrush(Colour::Khaki), 0.0f, 0.0f), Size(size)
{
	// Create asteroid geometry based on size wanted
	int minRadius = 40 / Size;
	int maxRadius = 60 / Size;
	int granularity = 20;
	int minVary = 25;
	int maxVary = 75;

	vector<D2D1_POINT_2F> points;

	for (double ang = 0; ang < 2 * M_PI; ang += 2 * M_PI / granularity)
	{
		int angleVaryPc = SimpleAsteroids::Random(minVary, maxVary);
		double angleVaryRadians = (2 * M_PI / granularity) * static_cast<double>(angleVaryPc) / 100;
		double angleFinal = ang + angleVaryRadians - (M_PI / granularity);

		int radius = SimpleAsteroids::Random(minRadius, maxRadius);

		float x = static_cast<float>(sin(angleFinal) * radius);
		float y = static_cast<float>(-cos(angleFinal) * radius);

		points.push_back(D2D1::Point2F(x, y));
	}

	// Finalize asteroid geometry
	GeometryData a = renderer->StartCreatePath(static_cast<int>(points[0].x), static_cast<int>(points[0].y), Geometry::Filled);
	a->AddLines(&(*points.begin()), points.size() - 1);
	geometry = renderer->EndCreatePath();

	// Set asteroid position, directional velocity and angular velocity
	x = (px == -1)? static_cast<float>(SimpleAsteroids::Random(0, renderer->ResolutionX)) : px;
	y = (py == -1)? static_cast<float>(SimpleAsteroids::Random(0, renderer->ResolutionY)) : py;

	angle = static_cast<float>(SimpleAsteroids::Random(0, 359));
	rotationalSpeed = static_cast<float>(SimpleAsteroids::Random(0, 100)) / 100.f - 0.5f;

	vX = (vx == -1)? static_cast<float>(SimpleAsteroids::Random(0, 100)) / 100.f - 0.5f : vx;
	vY = (vy == -1)? static_cast<float>(SimpleAsteroids::Random(0, 100)) / 100.f - 0.5f : vy;
}

// Create a new bullet
Bullet::Bullet(Simple2D *r, Player &p, float shipX, float shipY, float shipAngle) : GameWorldObject(r, r->MakeBrush(Colour::CornflowerBlue), 0.0f, 0.0f), player(p)
{
	// Create bullet geometry
	geometry = renderer->EllipseGeometry(1);

	// The direction of the bullet should be the direction in which the ship is facing
	vX = static_cast<float>(sin(2 * M_PI * (shipAngle / 360)));
	vY = static_cast<float>(-cos(2 * M_PI * (shipAngle / 360)));

	// The initial position of the bullet should be the ship's center point
	x = shipX;
	y = shipY;

	// Move the bullet along artificially a bit so it doesn't render inside the ship (we want it at the ship's front edge tip)
	x += vX * 20;
	y += vY * 20;

	// Bullet movement speed factor
	vX *= 5;
	vY *= 5;

	// No rotation or rotational speed
	angle = 0;
	rotationalSpeed = 0;

	// Bullet was created now
	createdTime = GetTickCount();

	// Bullet time-to-live
	ttl = 2000;
}

// Remove a bullet if its TTL has expired
bool Bullet::Remove()
{
	return (GetTickCount() - createdTime >= ttl);
}

// Delete a bullet
Bullet::~Bullet()
{
	player.EndFire();
}

SimpleAsteroids::SimpleAsteroids()
{
	// Set resolution
	SetResolution(800, 600);

	// New game world
	worldObjects.clear();

	// Game is not over
	gameOver = false;

	// Set up the player
	player = new Player(this);
	worldObjects.push_back(player);
	player->Reset();

	// Wave 1
	level = 1;
	CreateNewWave();
}

void SimpleAsteroids::CreateNewWave()
{
	// Set up asteroids (2 plus the level number)
	for (int i = 0; i < level + 2; i++)
		worldObjects.push_back(new Asteroid(this));
}

void SimpleAsteroids::UpdateObjects()
{
	// Don't do anything if the game is over
	if (gameOver)
		return;

	// Update game object positions
	for (auto it = worldObjects.begin(); it != worldObjects.end(); it++)
		it->Update();

	// Remove no longer needed objects
	for (auto it = worldObjects.begin(); it != worldObjects.end(); it++)
		if (it->Remove())
		{
			worldObjects.erase(it);
			it = worldObjects.begin();
		}

	// Check for collisions
	bool collision = false;

	for (auto it = worldObjects.begin(); it != worldObjects.end(); it++)
	{
		// Compare each bullet against each asteroid
		if (typeid(*it) == typeid(Bullet))
			for (auto it2 = worldObjects.begin(); it2 != worldObjects.end(); it2++)
				if (typeid(*it2) == typeid(Asteroid))
					if (it->IsCollision(*it2))
					{
						// Get asteroid object
						Asteroid &asteroid = *dynamic_cast<Asteroid *>(&(*it2));

						// Spawn two new asteroids if needed
						if (asteroid.Size < 3)
						{
							worldObjects.push_back(new Asteroid(this, asteroid.Size + 1, asteroid.GetX(), asteroid.GetY(), asteroid.GetVY() * 2, asteroid.GetVX() * 2));
							worldObjects.push_back(new Asteroid(this, asteroid.Size + 1, asteroid.GetX(), asteroid.GetY(), -asteroid.GetVY() * 2, -asteroid.GetVX() * 2));
						}

						// Update score
						switch (asteroid.Size) {
							case 1: player->Score += 10;
							case 2: player->Score += 50;
							case 3: player->Score += 100;
						}

						// Kill asteroid
						worldObjects.erase(it2);

						// Kill bullet
						worldObjects.erase(it);

						collision = true;
						break;
					}

		// Compare ship against each asteroid unless player is invincible
		if (!collision && !player->IsInvincible())
			if (typeid(*it) == typeid(Player))
				for (auto it2 = worldObjects.begin(); it2 != worldObjects.end(); it2++)
					if (typeid(*it2) == typeid(Asteroid))
						if (it->IsCollision(*it2))
						{
							// Kill player
							it->MakeInactiveFor(1000);

							// If they had no lives left, it's game over
							if (!player->Lives)
								gameOver = true;
						}

		if (collision)
			break;
	}

	// Count how many asterids are left. If zero, advance to the next wave
	int asteroidCount = 0;

	for (auto it = worldObjects.begin(); it != worldObjects.end(); it++)
		if (typeid(*it) == typeid(Asteroid))
			asteroidCount++;

	if (!asteroidCount)
	{
		level++;
		CreateNewWave();
	}

	// Check for player input
	if (GetAsyncKeyState(VK_LEFT))
		player->Rotate(-1);

	if (GetAsyncKeyState(VK_RIGHT))
		player->Rotate(1);

	if (GetAsyncKeyState(VK_UP))
		player->Accelerate();

	if (GetAsyncKeyState(' '))
	{
		Bullet *newBullet = player->Fire();

		if (newBullet)
			worldObjects.push_back(newBullet);
	}

	if (!GetAsyncKeyState(' '))
		player->ResetBulletCooldown();

	if (GetAsyncKeyState(VK_SHIFT))
		player->ActivateShield();
}

void SimpleAsteroids::DrawScene()
{
	// Draw the game world objects
	for (auto it = worldObjects.begin(); it != worldObjects.end(); it++)
		it->Draw();

	// Show current wave number
	Text(0, ResolutionY - 32, "Wave " + StringFactory(level), L"Verdana", 18.0f, Colour::White, DWRITE_TEXT_ALIGNMENT_TRAILING,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, ResolutionX - 20);

	// Print game over message if needed
	if (gameOver)
	{
		SetBrush(Colour::Black);
		CurrentBrush->SetOpacity(0.5f);
		FillRectangleWH(0, 0, ResolutionX, ResolutionY);

		Text(0, 236, "CEILING CAT", L"Courier New", 48.0f, Colour::LightGreen, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_FONT_WEIGHT_BOLD);
		Text(0, 275, "DECLINES YOUR REQUEST", L"Courier New", 48.0f, Colour::LightGreen, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_FONT_WEIGHT_BOLD);
		Text(0, 314, "FOR ADDITIONAL LIVES", L"Courier New", 48.0f, Colour::LightGreen, DWRITE_TEXT_ALIGNMENT_CENTER, DWRITE_FONT_WEIGHT_BOLD);
	}
}


// Application entry point
void Simple2DStart()
{
	SimpleAsteroids asteroids;
	asteroids.SetWindowName(L"SimpleAsteroids by Katy Coe (c) 2012");
	asteroids.SetBackgroundColour(Colour::Black);
	asteroids.Run();
}