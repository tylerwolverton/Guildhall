#include "ShaderCommon.hlsl"
#include "ShaderUtils.hlsl"
#include "LightingUtils.hlsl"
#include "PCUTBN_Common.hlsl"


//--------------------------------------------------------------------------------------
cbuffer material_constants : register( b5 )
{
	float ROOM_HEIGHT;
	float ROOM_WIDTH;
};


Texture2D <float4> tCeiling : register( t8 );
Texture2D <float4> tFloor : register( t9 );
Texture2D <float4> tWall : register( t10 );


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	v2f_t v2f = (v2f_t)0;

	// move the vertex through the spaces
	float4 localPos = float4( input.position, 1.0f );
	float4 worldPos = mul( MODEL, localPos );
	float4 cameraPos = mul( VIEW, worldPos );
	float4 clipPos = mul( PROJECTION, cameraPos );

	// tangent
	float4 localTangent = float4( input.tangent, 0.0f );
	float4 worldTangent = mul( MODEL, localTangent );
	
	// bitangent
	float4 localBitangent = float4( input.bitangent, 0.0f );
	float4 worldBitangent = mul( MODEL, localBitangent );

	// normal is currently in model/local space
	float4 localNormal = float4( input.normal, 0.0f );
	float4 worldNormal = mul( MODEL, localNormal );

	v2f.position = clipPos; // we want to output the clip position to raster (a perspective point)
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;

	v2f.world_position = worldPos.xyz;
	v2f.world_tangent = worldTangent.xyz;
	v2f.world_bitangent = worldBitangent.xyz;
	v2f.world_normal = worldNormal.xyz;

	return v2f;
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float3 view_dir = normalize( input.world_position - CAMERA_POSITION );

	float normalized_width_in_block = fmod( input.world_position.x, ROOM_WIDTH ) / ROOM_WIDTH;
	float normalized_height_in_block = fmod( input.world_position.y, ROOM_HEIGHT ) / ROOM_HEIGHT;

	float dot_wall_left = dot( view_dir, float3( 1.f, 0.f, 0.f ) );
	float dot_wall_right = dot( view_dir, float3( -1.f, 0.f, 0.f ) );
	float dot_ceiling = dot( view_dir, float3( 0.f, -1.f, 0.f ) );
	float dot_floor = dot( view_dir, float3( 0.f, 1.f, 0.f ) );
	float dot_wall_back = dot( view_dir, float3( 0.f, 0.f, 1.f ) );

	float dist_to_wall_left  = 999999.f;
	float dist_to_wall_right = 999999.f;
	float dist_to_ceiling    = 999999.f;
	float dist_to_floor      = 999999.f;
	float dist_to_wall_back  = 999999.f;

	if ( dot_wall_left < 0.f )
	{

	}

	// use the uv to sample the texture
	float4 ceiling_color = tCeiling.Sample( sSampler, input.uv );
	float4 floor_color = tFloor.Sample( sSampler, input.uv );
	float4 wall_color = tWall.Sample( sSampler, input.uv );

	
	float3 final_color = ceiling_color.xyz;

	return float4( final_color, 1.f );
}