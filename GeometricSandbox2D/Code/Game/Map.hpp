#pragma once
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
class Polygon2;


//-----------------------------------------------------------------------------------------------
enum class eBroadphaseCheckType
{
	NONE,
	QUAD_TREE
};


//-----------------------------------------------------------------------------------------------
enum class eNarrowphaseCheckType
{
	NONE,
	BOUNDING_DISC,
};


//-----------------------------------------------------------------------------------------------
struct RaycastResult
{
	Vec2 startPos;
	Vec2 forwardNormal;
	float maxDist = 0.f;
	bool didImpact = false;
	Vec2 impactPos;
	Entity* impactEntity = nullptr;
	float impactFraction = 0.f;
	float impactDist = 0.f;
	Vec2 impactSurfaceNormal;
};


//-----------------------------------------------------------------------------------------------
class Map
{
public:
	Map( const std::string& name );
	~Map();

	virtual void Load();
	virtual void Unload();

	virtual void Reset();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void DebugRender() const;

	virtual std::string GetName()													{ return m_name; }

	virtual void AddEntitySet( int numEntities );
	virtual void RemoveEntitySet( int numEntities );
	virtual Entity* SpawnNewEntity( const Polygon2& polygon );

	virtual int GetObjectCount() const;
	virtual int GetRaycastCount() const;

	virtual void CycleBroadphaseCheck();
	virtual void CycleNarrowphaseCheck();

	double GetRaycastTimeMs() const													{ return m_raycastTimeMs; }
	int GetNumRaycastImpacts() const												{ return m_numRaycastImpacts; }

	virtual eBroadphaseCheckType GetBroadphaseCheckType() const						{ return m_broadphaseCheckType; }
	virtual std::string GetBroadphaseCheckTypeStr() const;
	virtual eNarrowphaseCheckType GetNarrowphaseCheckType() const					{ return m_narrowphaseCheckType; }
	virtual std::string GetNarowphaseCheckTypeStr() const;

protected:		
	virtual RaycastResult Raycast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist );

protected:
	std::string			 m_name;
	std::vector<Entity*> m_entities;

	eBroadphaseCheckType m_broadphaseCheckType = eBroadphaseCheckType::NONE;
	eNarrowphaseCheckType m_narrowphaseCheckType = eNarrowphaseCheckType::NONE;

	double				 m_raycastTimeMs = 0.0;
	int					 m_numRaycastImpacts = 0;
};
