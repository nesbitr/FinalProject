#include <boost/ptr_container/ptr_list.hpp>

using namespace S2D;
using std::vector;

class Player;

// ==========================================================================
// Generic game world object
// ==========================================================================

class GameWorldObject
{
protected:
	// Pointer to renderer
	Simple2D *renderer;

	// Set to true if the object should be rendered and checked for collisions
	bool active;

	// Time to re-activate if object is inactive
	unsigned int reactivateTime;

	// The object's center position in the game world
	float x, y;

	// The object's current angle of rotation
	float angle;

	// The current velocity vector of the object
	float vX, vY;

	// The drag co-efficient acting on the object
	float dragFactor;

	// The rotational speed of the object (degrees per frame)
	float rotationalSpeed;

	// The object geometry
	Geometry geometry;

	// The outline geometry brush
	Paintbrush outlineBrush;

public:
	// Constructors
	GameWorldObject(Simple2D *r, Paintbrush oB, float dF, float rF) : renderer(r), outlineBrush(oB), dragFactor(dF), rotationalSpeed(rF), active(true) {}

	// Virtual destructor needed for correct deletion of objects from a boost::ptr_vector
	virtual ~GameWorldObject() {}

	// Update object position and status
	virtual void Update();

	// Draw the object in the game world
	virtual void Draw();

	// Return true if the object should be removed from the game world
	virtual bool Remove() { return false; }

	// Return true if two geometries intersect
	bool IsCollision(GameWorldObject &o);

	// Make object inactive for X ms
	void MakeInactiveFor(int);

	// Reset an object after inactivity
	virtual void Reset() {}
};

// ==========================================================================
// An asteroid
// ==========================================================================

class Asteroid : public GameWorldObject
{
public:
	// Constructor
	Asteroid(Simple2D *r, int size = 1, float px = -1, float py = -1, float vx = -1, float vy = -1);

	// Size of asteroid (1-3)
	int Size;

	// Get asteroid co-ordinates
	float GetX() { return x; }
	float GetY() { return y; }
	float GetVX() { return vX; }
	float GetVY() { return vY; }
};

// ==========================================================================
// A bullet
// ==========================================================================

class Bullet : public GameWorldObject
{
	// The ship which owns this bullet
	Player &player;

	// How long the bullet stays active in milliseconds
	unsigned int ttl;

	// When the bullet entered the game world
	unsigned int createdTime;

public:
	// Constructor
	Bullet(Simple2D *r, Player &p, float sX, float sY, float sA);

	// Destructor
	virtual ~Bullet();

	// Implementation
	virtual bool Remove();
};

// ==========================================================================
// Player's spaceship
// ==========================================================================

class Player : public GameWorldObject
{
	// The acceleration factor of the ship
	float accFactor;

	// The rotation factor of the ship
	float rotFactor;

	// Number of bullets currently in use
	int bulletsUsed;

	// Max number of allowed bullets
	int maxBullets;

	// Minimum time between each bullet (ms)
	unsigned int bulletCooldown;

	// Time last bullet was fired
	unsigned int lastBulletTime;

	// Currently invincible
	bool invincibility;

	// Time invincibility lasts (ms)
	unsigned int shieldDuration;

	// Minimum time between each shield use (ms)
	unsigned int shieldCooldown;

	// Time shield was last used
	unsigned int lastShieldTime;

public:
	// Current lives remaining
	int Lives;

	// Current score
	int Score;

	// Constructor
	Player(Simple2D *r);

	// Reset the ship to its default settings
	virtual void Reset();

	// Rotate the ship (user interaction)
	void Rotate(int);

	// Accelerate in the direction currently facing (user interaction)
	void Accelerate();

	// Create a new bullet and return a pointer to it
	Bullet *Fire();

	// Release one bullet slot
	void EndFire();

	// Allow a new bullet to be fired immediately if any slots free
	void ResetBulletCooldown();

	// Update movement, status etc.
	virtual void Update();

	// Draw the lives, score etc.
	virtual void Draw();

	// Activate invincibility if cooldown has passed
	void ActivateShield();

	// Return true if player is currently invincible
	bool IsInvincible() { return invincibility; }
};

// ==========================================================================
// Application class
// ==========================================================================

class SimpleAsteroids : public Simple2D
{
public:
	// Constructor
	SimpleAsteroids();

private:
	// Callbacks
	void UpdateObjects();
	void DrawScene();

	// Create next wave of asteroids
	void CreateNewWave();

	// The game world objects
	boost::ptr_list<GameWorldObject> worldObjects;

	// Pointer to player
	Player *player;

	// Current wave
	int level;

	// Game over state
	bool gameOver;
};