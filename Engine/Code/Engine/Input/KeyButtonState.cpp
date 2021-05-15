#include "Engine/Input/KeyButtonState.hpp"


//-----------------------------------------------------------------------------------------------
void KeyButtonState::UpdateStatus( bool isNowPressed )
{
	m_wasPressedLastFrame = m_isPressed;
	m_isPressed = isNowPressed;

	if ( isNowPressed )
	{
		++m_numTimesPressed;
	}
	else if ( !m_isPressed && m_wasPressedLastFrame )
	{
		++m_numTimesReleased;
	}
}


//-----------------------------------------------------------------------------------------------
bool KeyButtonState::WasJustPressed() const
{
	if ( m_isPressed && !m_wasPressedLastFrame )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool KeyButtonState::WasJustReleased() const
{
	if ( !m_isPressed && m_wasPressedLastFrame )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void KeyButtonState::Reset()
{
	m_isPressed = false;
	m_wasPressedLastFrame = false;
	m_numTimesPressed = 0;
	m_numTimesReleased = 0;
}


//-----------------------------------------------------------------------------------------------
bool KeyButtonState::ConsumeKeyPress()
{
	if ( m_isPressed
		 && m_numTimesPressed > 0 )
	{
		--m_numTimesPressed;

		if ( m_numTimesPressed == 0 )
		{
			m_isPressed = false;
		}

		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
int KeyButtonState::ConsumeAllKeyPresses()
{
	int numPresses = m_numTimesPressed;
	m_numTimesPressed = 0;
	m_isPressed = false;
	return numPresses;
}


//-----------------------------------------------------------------------------------------------
int KeyButtonState::ConsumeAllKeyReleases()
{
	int numReleases = m_numTimesReleased;
	m_numTimesReleased = 0;
	m_isPressed = false;
	return numReleases;
}
