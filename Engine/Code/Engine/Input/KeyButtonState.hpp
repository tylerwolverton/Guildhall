#pragma once


//-----------------------------------------------------------------------------------------------
class KeyButtonState
{
public:
	void	UpdateStatus(bool isNowPressed);
	bool	IsPressed() const					{ return m_isPressed; }
	bool	WasJustPressed() const;
	bool	WasJustReleased() const;
	void	Reset();

	bool	ConsumeKeyPress();
	bool	ConsumeKeyRelease();
	int		ConsumeAllKeyPresses();
	int		ConsumeAllKeyReleases();

private:
	bool m_isPressed = false;
	bool m_wasPressedLastFrame = false;

	int m_numTimesPressed = 0;
	int m_numTimesReleased = 0;
};
