#pragma once
#include "Common.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec2;


//-----------------------------------------------------------------------------------------------
void AppendVertsForLine( std::vector<VertexPC>& verts, const TileCoords& startTile, const TileCoords& endTile, float thickness, Color8 color );
void AppendVertsForLine( std::vector<VertexPC>& verts, const Vec2& startPos, const Vec2& endPos, float thickness, Color8 color );
void AppendVertsForTile( std::vector<VertexPC>& verts, const TileCoords& tileCoords, Color8 color );
void AppendVertsForTileOutline( std::vector<VertexPC>& verts, const TileCoords& tileCoords, float thickness, Color8 color );

void PushVertex( std::vector<VertexPC>& verts, const Vec2& pos, const Color8& color );
