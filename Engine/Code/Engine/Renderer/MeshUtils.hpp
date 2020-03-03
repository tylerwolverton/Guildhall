#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>


struct Rgba8;
struct Vertex_PCU;
struct AABB2;
struct OBB2;
struct Capsule2;
class Polygon2;


void AppendVertsForArc( std::vector<Vertex_PCU>& vertexArray, const Vec2& center, float radius, float arcAngleDegrees, float startOrientationDegrees, const Rgba8& tint );
void AppendVertsForAABB2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& spriteBounds, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const OBB2& spriteBounds, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& bottomLeft, const Vec2& bottomRight, const Vec2& topLeft, const Vec2& topRight, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForCapsule2D( std::vector<Vertex_PCU>& vertexArray, const Capsule2& capsule, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForPolygon2( std::vector<Vertex_PCU>& vertexArray, const std::vector<Vec2>& vertexPositions, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );

void AppendVertsForAABB2DWithDepth( std::vector<Vertex_PCU>& vertexArray, const AABB2& spriteBounds, float zDepth, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );

void AppendVertsForCubeMesh( std::vector<Vertex_PCU>& vertexArray, const Vec3& center, float sideLength, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendIndicesForCubeMesh( std::vector<uint>& indices );