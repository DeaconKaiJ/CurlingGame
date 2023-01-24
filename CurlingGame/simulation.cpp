/*-----------------------------------------------------------
	Simulation Source File
-----------------------------------------------------------*/
#include"simulation.h"
#include<stdio.h>
#include <iostream>
#include <algorithm>

/*-----------------------------------------------------------
	macros
-----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.01f)

game gGame;

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;

/*-----------------------------------------------------------
	players class members
-----------------------------------------------------------*/
int players::playerIndexCnt = 0;

/*-----------------------------------------------------------
	target class members
-----------------------------------------------------------*/
int ScoreGrid::sgIndexCnt = 0;
void ScoreGrid::radiusD(void)
{
	radius = STONE_RADIUS * (index + 1); //calculate radius based on index
}

/*-----------------------------------------------------------
	walls class members
-----------------------------------------------------------*/
void walls::MakeNormal(void)
{
	//normalise
	vec2 temp = vertices[1] - vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void walls::MakeCentre(void)
{
	//find center
	centre = vertices[0];
	centre += vertices[1];
	centre /= 2.0;
}

/*-----------------------------------------------------------
	slate class members
-----------------------------------------------------------*/
int slateController::slateIndexCnt = 0;

void slateController::SetupCushions(void)
{
	//calculate middle point of new slate
	float moveRight;
	float separate;
	moveRight = SLATE_X * 2 * num;
	separate = 0.10 * num;
	floor = moveRight + separate;

	//Left
	slateWalls[0] = new walls;
	slateWalls[0]->vertices[0](0) = -SLATE_X + floor;
	slateWalls[0]->vertices[0](1) = -SLATE_Z;
	slateWalls[0]->vertices[1](0) = -SLATE_X + floor;
	slateWalls[0]->vertices[1](1) = SLATE_Z;

	//Bottom
	slateWalls[1] = new walls;
	slateWalls[1]->vertices[0](0) = -SLATE_X + floor;
	slateWalls[1]->vertices[0](1) = SLATE_Z;
	slateWalls[1]->vertices[1](0) = SLATE_X + floor;
	slateWalls[1]->vertices[1](1) = SLATE_Z;

	//Right
	slateWalls[2] = new walls;
	slateWalls[2]->vertices[0](0) = SLATE_X + floor;
	slateWalls[2]->vertices[0](1) = SLATE_Z;
	slateWalls[2]->vertices[1](0) = SLATE_X + floor;
	slateWalls[2]->vertices[1](1) = -SLATE_Z;

	//Top
	slateWalls[3] = new walls;
	slateWalls[3]->vertices[0](0) = SLATE_X + floor;
	slateWalls[3]->vertices[0](1) = -SLATE_Z;
	slateWalls[3]->vertices[1](0) = -SLATE_X + floor;
	slateWalls[3]->vertices[1](1) = -SLATE_Z;

	for (int i = 0; i < NUM_WALLS; i++)
	{
		slateWalls[i]->MakeCentre();
		slateWalls[i]->MakeNormal();
	}
}

/*-----------------------------------------------------------
	stone class members
-----------------------------------------------------------*/
int stone::ballIndexCnt = 0;

void stone::ApplyImpulse(vec2 imp)
{
	//Apply movement, change has been hit to true
	velocity = imp;
	impTrue = true;
}

void stone::ApplyFrictionForce(int ms)
{
	if (velocity.Magnitude() <= 0.0) return;

	//accelaration is opposite to direction of motion
	vec2 accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2 velocityChange = ((accelaration * ms) / 1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if (velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;
}

void stone::DoBallCollision(stone& b)
{
	//test if ball has hit another ball
	if (HasHitBall(b)) HitBall(b);
}

void stone::DoPlaneCollision(const walls& b)
{
	//test if ball has hit plane
	if (HasHitPlane(b)) HitPlane(b);
}

void stone::Update(int ms)
{
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms) / 1000.0f);
	//set small velocities to zero
	if (velocity.Magnitude() < SMALL_VELOCITY) velocity = 0.0;
}


bool stone::HasHitPlane(const walls& c) const
{
	//if moving away from plane, cannot hit
	if (velocity.Dot(c.normal) >= 0.0) return false;

	//if in front of plane, then have not hit
	vec2 relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);
	if (sep > radius) return false;
	return true;
}

bool stone::HasHitBall(const stone& b) const
{
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2 relPosn = position - b.position;
	float dist = (float)relPosn.Magnitude();
	vec2 relPosnNorm = relPosn.Normalised();
	vec2 relVelocity = velocity - b.velocity;

	//if moving apart, cannot have hit
	if (relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if (dist > (radius + b.radius)) return false;
	return true;
}

void stone::HitPlane(const walls& c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0 + gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta;
}

void stone::HitBall(stone& b)
{
	//find direction from other ball to this ball
	vec2 relDir = (position - b.position).Normalised();

	//split velocities into 2 parts:  one component perpendicular, and one parallel to 
	//the collision plane, for both balls
	//(NB the collision plane is defined by the point of contact and the contact normal)
	float perpV = (float)velocity.Dot(relDir);
	float perpV2 = (float)b.velocity.Dot(relDir);
	vec2 parallelV = velocity - (relDir * perpV);
	vec2 parallelV2 = b.velocity - (relDir * perpV2);

	//Calculate new perpendicluar components:
	//v1 = (2*m2 / m1+m2)*u2 + ((m1 - m2)/(m1+m2))*u1;
	//v2 = (2*m1 / m1+m2)*u1 + ((m2 - m1)/(m1+m2))*u2;
	float sumMass = mass + b.mass;
	float perpVNew = (float)((perpV * (mass - b.mass)) / sumMass) + (float)((perpV2 * (2.0 * b.mass)) / sumMass);
	float perpVNew2 = (float)((perpV2 * (b.mass - mass)) / sumMass) + (float)((perpV * (2.0 * mass)) / sumMass);

	//find new velocities by adding unchanged parallel component to new perpendicluar component
	velocity = parallelV + (relDir * perpVNew);
	b.velocity = parallelV2 + (relDir * perpVNew2);
}

/*-----------------------------------------------------------
	stone controller class members
-----------------------------------------------------------*/
void curlingStones::AddBall(const vec2& pos)
{
	//Test if all stones have been made, start game end
	if (num >= NUM_STONES) gameEnd = true;
	if (num >= NUM_STONES) return;
	//make new stone with given position
	stones[num] = new stone;
	stones[num]->position = pos;
	num++;
}
void curlingStones::gReset(void)
{
	//delete all stones
	int i = 0;
	while (i < num)
	{
		delete stones[i];
		stones[i] = stones[num - 1];
		num--;
	}
	gameEnd = false;
}


/*-----------------------------------------------------------
	firwork class members
-----------------------------------------------------------*/
 
void firework::update(int ms)
{
	//update position and veloctity
	position += (velocity * ms) / 1000.0;
	velocity(1) -= (4.0 * ms) / 1000.0;
}

/*-----------------------------------------------------------
	firework set class members
-----------------------------------------------------------*/
void fireworkSet::AddFirework(const vec3& pos)
{
	//test if num will exceed maximum particles
	if (num >= MAX_PARTICLES) return;
	//make new firework at position with random trajectory
	fireworks[num] = new firework;
	fireworks[num]->position = pos;

	fireworks[num]->velocity(0) = ((rand() % 200) - 100) / 200.0;
	fireworks[num]->velocity(2) = ((rand() % 200) - 100) / 200.0;
	fireworks[num]->velocity(1) = 2.0 * ((rand() % 100) / 100.0);

	num++;
}

void fireworkSet::update(int ms)
{
	//delete firework if below the floor
	int i = 0;
	while (i < num)
	{
		fireworks[i]->update(ms);
		if ((fireworks[i]->position(1) < 0.0) && (fireworks[i]->velocity(1) < 0.0))
		{
			delete fireworks[i];
			fireworks[i] = fireworks[num - 1];
			num--;
		}
		else i++;
	}
}

/*-----------------------------------------------------------
	Game class members
-----------------------------------------------------------*/
void game::Update(int ms, int activeSlate)
{
	//check for collisions for each ball
	for (int i = 0; i < gGame.cs.num; i++)
	{
		for (int j = 0; j < NUM_WALLS; j++)
		{			
			gGame.cs.stones[i]->DoPlaneCollision(*gGame.slate[activeSlate].slateWalls[j]);
		}

		for (int j = (i + 1); j < gGame.cs.num; j++)
		{
			gGame.cs.stones[i]->DoBallCollision(*gGame.cs.stones[j]);
		}
	}

	for (int i = 0; i < gGame.cs.num; i++) gGame.cs.stones[i]->Update(ms);

	fworks.update(ms);
}

bool game::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for (int i = 0; i < gGame.cs.num; i++)
	{
		if (gGame.cs.stones[i]->velocity(0) != 0.0) return true;
		if (gGame.cs.stones[i]->velocity(1) != 0.0) return true;
	}
	return false;
}

int* game::calculateScore(float x)
{
	//return array
	static int returnBallID[NUM_STONES];

	//variables for calculating distances
	int ballID[NUM_STONES];
	float allDist[NUM_STONES];
	float tempDist[NUM_STONES];

	for (int i = 0; i < NUM_STONES; i++)
	{
		//find distance
		float currentDistance = sqrt(pow(cs.stones[i]->position(0) - x, 2.0) + pow(cs.stones[i]->position(1) - -0.8, 2.0));
		//append i and distance to respective arrays
		ballID[i] = i, tempDist[i] = currentDistance, allDist[i] = currentDistance;
	}

	//sort distances
	std::sort(tempDist, tempDist + NUM_STONES);

	//itertate back through both arrays to find matching values and sort ids
	for (int i = 0; i < NUM_STONES; i++)
	{
		for (int j = 0; j < NUM_STONES; j++)
		{
			if (tempDist[i] == allDist[j]) returnBallID[i] = ballID[j];
		}
	}
	return returnBallID;
}
float* game::calcCam(int currentPlayer, float pos, float r)
{
	static float returnXZ[2];
	float ballX = cs.stones[currentPlayer]->position(0);
	float ballZ = cs.stones[currentPlayer]->position(1);
	float theta = TWO_PI * float(pos) / float(360);
	returnXZ[0] = r * cosf(theta) + ballX;//calculate the x component 
	returnXZ[1] = r * sinf(theta) + ballZ;//calculate the z component
	return returnXZ;
}