#include "ShaderCommon.hlsl"
#include "ShaderUtils.hlsl"
#include "LightingUtils.hlsl"
#include "PCUTBN_Common.hlsl"


cbuffer material_constants : register( b5 )
{
	float4x4 PROJECTION_MATRIX;

	float3 PROJECTOR_POSITION;
	float PROJECTOR_POWER;
};


Texture2D <float4> tImage : register( t8 );

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


	// normal is currently in model/local space
	float4 localNormal = float4( input.normal, 0.0f );
	float4 worldNormal = mul( MODEL, localNormal );

	v2f.position = clipPos; // we want to output the clip position to raster (a perspective point)
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;

	v2f.world_position = worldPos.xyz;
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
	float4 clip_pos = mul( float4( input.world_position, 1.f ), PROJECTION_MATRIX );
	//float z_local = clip_pos.w;

	float3 ndc = clip_pos.xyz / clip_pos.w;
	float2 uv = ( ndc.xy + float2( 1.f, 1.f ) * .5f );

	// 1 when inside 0-1, 0 otherwise
	float u_blend = step( 0.f, uv.x ) * ( 1.f - step( 1.f, uv.x ) );
	float v_blend = step( 0.f, uv.y ) * ( 1.f - step( 1.f, uv.y ) );
	float blend = u_blend * v_blend;

	float4 texture_color = tImage.Sample( sSampler, uv );

	float3 dir_to_camera = normalize( PROJECTOR_POSITION - input.world_position );
	float3 world_normal = normalize( input.world_normal );

	float facing = step( 0.f, max( 0.f, dot( dir_to_camera, world_normal ) ) );
	blend *= facing;

	blend *= step( 0.f, ndc.z ); // maybe 1.f - 

	float4 final_color = lerp( 0.f.xxxx, texture_color, blend );
	return final_color * PROJECTOR_POWER;
}