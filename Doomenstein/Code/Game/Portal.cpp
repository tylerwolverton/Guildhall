#include "Game/Portal.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Portal::Portal( const EntityDefinition& entityDef )
	: Entity( entityDef )
{

}


//-----------------------------------------------------------------------------------------------
Portal::~Portal()
{
}


//-----------------------------------------------------------------------------------------------
void Portal::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Portal::Render() const
{
	
}


//-----------------------------------------------------------------------------------------------
void Portal::Die()
{
	Entity::Die();
}
