#include "RenderUtils.hpp"
#include "Math/Vec2.hpp"
#include "Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
void AppendVertsForLine( std::vector<VertexPC>& verts, const TileCoords& startTile, const TileCoords& endTile, float thickness, Color8 color )
{
	AppendVertsForLine( verts, Vec2( (float)startTile.x, (float)startTile.y ), Vec2( (float)endTile.x, (float)endTile.y ), thickness, color );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForLine( std::vector<VertexPC>& verts, const Vec2& startPos, const Vec2& endPos, float thickness, Color8 color )
{
	Vec2 forwardVec = GetNormalizedDirectionFromAToB( startPos, endPos );
	Vec2 leftVec = forwardVec.GetRotated90Degrees();

	Vec2 topLeft		= startPos + ( leftVec * thickness * .5f );
	Vec2 bottomLeft		= startPos - ( leftVec * thickness * .5f );
	Vec2 bottomRight	= endPos - ( leftVec * thickness * .5f );
	Vec2 topRight		= endPos + ( leftVec * thickness * .5f );

	PushVertex( verts, topLeft, color );
	PushVertex( verts, bottomLeft, color );
	PushVertex( verts, bottomRight, color );

	PushVertex( verts, bottomLeft, color );
	PushVertex( verts, bottomRight, color );
	PushVertex( verts, topRight, color );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForTile( std::vector<VertexPC>& verts, const TileCoords& tileCoords, Color8 color )
{

}


//-----------------------------------------------------------------------------------------------
void AppendVertsForTileOutline( std::vector<VertexPC>& verts, const TileCoords& tileCoords, float thickness, Color8 color )
{
	Vec2 tileCenter( (float)tileCoords.x, (float)tileCoords.y );
	float tileWidth = .5f;

	Vec2 topLeft =		tileCenter + Vec2( -tileWidth, tileWidth );
	Vec2 bottomLeft =	tileCenter + Vec2( -tileWidth, -tileWidth );
	Vec2 bottomRight =	tileCenter + Vec2( tileWidth, -tileWidth );
	Vec2 topRight =		tileCenter + Vec2( tileWidth, tileWidth );

	AppendVertsForLine( verts, topLeft,		topRight, thickness, color );
	AppendVertsForLine( verts, bottomLeft,	bottomRight, thickness, color );
	AppendVertsForLine( verts, topLeft,		bottomLeft, thickness, color );
	AppendVertsForLine( verts, topRight,	bottomRight, thickness, color );
}


//-----------------------------------------------------------------------------------------------
void PushVertex( std::vector<VertexPC>& verts, const Vec2& pos, const Color8& color )
{
	VertexPC vert;
	vert.x = pos.x;
	vert.y = pos.y;
	vert.rgba = color;

	verts.emplace_back( vert );
}
