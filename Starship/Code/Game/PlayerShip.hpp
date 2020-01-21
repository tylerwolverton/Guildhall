#pragma once
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class PlayerShip :
	public Entity
{
public:
	PlayerShip(Game* theGame, const Vec2& position);
	virtual ~PlayerShip() {}

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void Die() override;

	void RenderRemainingLives() const;
	void RenderCooldowns() const;
	
	// State update methods to respond to input
	void AccelerateShip();
	void StopAcceleratingShip();

	void TurnShipLeft()				{ m_isShipTurningLeft = true; }
	void TurnShipRight()			{ m_isShipTurningRight = true; }
	void StopTurningShipLeft()		{ m_isShipTurningLeft = false; }
	void StopTurningShipRight()		{ m_isShipTurningRight = false; }
	
	void HandleBulletFired();
	void HandleStarburstBulletFired();
	void HandleBulletReload()			{ m_canFireBullet = true; };

	void Respawn();

private:
	void UpdateFromKeyboard	( float deltaSeconds );
	void UpdateFromGamepad	( float deltaSeconds );

	// Movement
	void TurnAndThrust( float deltaSeconds );
	void CheckForWallBounce();

private:
	bool m_isShipAccelerating = false;
	bool m_isShipAcceleratingFromJoystick = false;
	bool m_isShipTurningLeft = false;
	bool m_isShipTurningRight = false;
	bool m_canFireBullet = true;

	int	  m_controllerID = 0;
	float m_thrustFraction = 0.f;

	float m_vibrationSeconds = 0.f;
	float m_leftVibrationFraction = 0.f;
	float m_rightVibrationFraction = 0.f;

	int m_remainingLives = 4;

	bool	m_canFireStarburstBullet = true;
	float	m_starburstCooldownSeconds = 0.f;
	unsigned char m_starburstCooldownAlpha = (unsigned char)255;
};

