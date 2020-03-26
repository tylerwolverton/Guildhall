#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Rgba8;
struct Vertex_PCU;
struct AABB2;
struct AABB3;
struct OBB2;
struct OBB3;
struct Capsule2;
class Polygon2;
class RenderContext;


//-----------------------------------------------------------------------------------------------
void DrawLine2D( RenderContext* renderer, const Vec2& start, const Vec2& end, const Rgba8& color, float thickness );
void DrawRing2D( RenderContext* renderer, const Vec2& center, float radius, const Rgba8& color, float thickness );
void DrawDisc2D( RenderContext* renderer, const Vec2& center, float radius, const Rgba8& color );
void DrawCapsule2D( RenderContext* renderer, const Capsule2& capsule, const Rgba8& color );
void DrawAABB2( RenderContext* renderer, const AABB2& box, const Rgba8& tint );
void DrawAABB2Outline( RenderContext* renderer, const Vec2& center, const AABB2& box, const Rgba8& tint, float thickness );
void DrawOBB2( RenderContext* renderer, const OBB2& box, const Rgba8& tint );
void DrawOBB2Outline( RenderContext* renderer, const Vec2& center, const OBB2& box, const Rgba8& tint, float thickness );
void DrawPolygon2( RenderContext* renderer, const Polygon2& polygon2, const Rgba8& tint );
void DrawPolygon2( RenderContext* renderer, const std::vector<Vec2>& vertexPositions, const Rgba8& tint );
void DrawPolygon2Outline( RenderContext* renderer, const Polygon2& polygon2, const Rgba8& tint, float thickness );
void DrawPolygon2Outline( RenderContext* renderer, const std::vector<Vec2>& vertexPositions, const Rgba8& tint, float thickness );

void DrawAABB2WithDepth( RenderContext* renderer, const AABB2& box, float zDepth, const Rgba8& tint );

//-----------------------------------------------------------------------------------------------
// Append vertices to given vector
void AppendVertsForArc( std::vector<Vertex_PCU>& vertexArray, const Vec2& center, float radius, float arcAngleDegrees, float startOrientationDegrees, const Rgba8& tint );
void AppendVertsForAABB2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& spriteBounds, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const OBB2& spriteBounds, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& bottomLeft, const Vec2& bottomRight, const Vec2& topLeft, const Vec2& topRight, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForCapsule2D( std::vector<Vertex_PCU>& vertexArray, const Capsule2& capsule, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForPolygon2( std::vector<Vertex_PCU>& vertexArray, const std::vector<Vec2>& vertexPositions, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );

void AppendVertsForAABB2DWithDepth( std::vector<Vertex_PCU>& vertexArray, const AABB2& spriteBounds, float zDepth, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForAABB3D( std::vector<Vertex_PCU>& vertexArray, const AABB3& bounds, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForOBB3D( std::vector<Vertex_PCU>& vertexArray, const OBB3& bounds, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForLine3D( std::vector<Vertex_PCU>& vertexArray, const Vec3& p0, const Vec3& p1, const Rgba8& p0_start_color, const Rgba8& p1_start_color, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );

void AppendVertsForCubeMesh( std::vector<Vertex_PCU>& vertexArray, const Vec3& center, float sideLength, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsFor3DBox( std::vector<Vertex_PCU>& vertexArray, std::vector<Vec3>& corners, const Rgba8& frontTint, const Rgba8& backTint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsFor3DBox( std::vector<Vertex_PCU>& vertexArray, int cornerCount, Vec3* corners, const Rgba8& frontTint, const Rgba8& backTint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendIndicesForCubeMesh( std::vector<uint>& indices );

void AppendVertsForPlaneMesh( std::vector<Vertex_PCU>& vertexArray, const Vec3& mins, const Vec2& dimensions, int horizontalSlices, int verticalSlices, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendIndicesForPlaneMesh( std::vector<uint>& indices, int horizontalSlices, int verticalSlices );

void AppendVertsAndIndicesForSphereMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, const Vec3& center, float radius, int horizontalSlices, int verticalSlices, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsAndIndicesForCylinderMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, const Vec3& p0, const Vec3& p1, float radius1, float radius2, const Rgba8& tint, int numSides = 8, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsAndIndicesForConeMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, const Vec3& p0, const Vec3& p1, float radius, const Rgba8& tint, int numSides = 8, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
