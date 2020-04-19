#pragma once
#include "Game/GameObject.hpp"

#include <string>

class InteractableSwitch : public GameObject
{
public:
	InteractableSwitch( GameObject* player, float radius );

	virtual void Update() override;

	void Enable();
	void Activate();

	bool IsEnabled()											{ return m_isEnabled; }
	
private:
	GameObject* m_player = nullptr;
	float m_radius = 0.f;

	std::string m_helpText = "Press E to interact";

	bool m_isEnabled = false;
};