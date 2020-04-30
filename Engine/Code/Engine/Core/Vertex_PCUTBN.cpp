#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------------------------
const BufferAttribute Vertex_PCUTBN::LAYOUT[] =
{
   BufferAttribute( "POSITION",   BUFFER_FORMAT_VEC3,      		offsetof( Vertex_PCUTBN, position ) ),
   BufferAttribute( "COLOR",      BUFFER_FORMAT_R8G8B8A8_UNORM, offsetof( Vertex_PCUTBN, color ) ),
   BufferAttribute( "TEXCOORD",   BUFFER_FORMAT_VEC2,			offsetof( Vertex_PCUTBN, uvTexCoords ) ),
   BufferAttribute( "TANGENT",    BUFFER_FORMAT_VEC3,      		offsetof( Vertex_PCUTBN, tangent ) ),
   BufferAttribute( "BITANGENT",  BUFFER_FORMAT_VEC3,      		offsetof( Vertex_PCUTBN, bitangent ) ),
   BufferAttribute( "NORMAL",     BUFFER_FORMAT_VEC3,      		offsetof( Vertex_PCUTBN, normal ) ),
   BufferAttribute() // end - terminator element; 
};


//-----------------------------------------------------------------------------------------------
Vertex_PCUTBN::Vertex_PCUTBN( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec3& normal, const Vec3& tangent )
	: position( position )
	, color( tint )
	, uvTexCoords( uvTexCoords )
	, normal( normal )
	, tangent( tangent )
{
	bitangent = CrossProduct3D( normal, tangent );
}


//-----------------------------------------------------------------------------------------------
bool Vertex_PCUTBN::operator==( const Vertex_PCUTBN& other )
{
	if ( !IsNearlyEqual( position, other.position ) )
	{
		return false;
	}

	if ( color != other.color )
	{
		return false;
	}

	if ( !IsNearlyEqual( uvTexCoords, other.uvTexCoords ) )
	{
		return false;
	}

	if ( !IsNearlyEqual( normal, other.normal ) )
	{
		return false;
	}

	if ( !IsNearlyEqual( tangent, other.tangent ) )
	{
		return false;
	}

	if ( !IsNearlyEqual( bitangent, other.bitangent ) )
	{
		return false;
	}

	return true;
}

