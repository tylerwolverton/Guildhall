#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Rgba8;
struct Vertex_PCU;


//-----------------------------------------------------------------------------------------------
void AppendVertsAndIndicesForCubeMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, const Vec3& center, float sideLength, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendVertsForCubeMesh( std::vector<Vertex_PCU>& vertexArray, const Vec3& center, float sideLength, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
void AppendIndicesForCubeMesh( std::vector<uint>& indices, uint startIdx = 0 );
