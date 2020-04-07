#include "ShaderCommon.hlsl"
#include "PCUTBN_Common.hlsl"


// Textures & Samplers are also a form of constant
// data - uniform/constant across the entire call
Texture2D <float4> tDiffuse   : register( t0 );   // color of the surface
SamplerState sSampler : register( s0 );           // sampler are rules on how to sample (read) from the texture.


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
	float3 surface_normal = normalize( input.world_normal );

	float3 final_color =  ( surface_normal + float3( 1.f, 1.f, 1.f ) ) * .5f;

	return float4( final_color, 1.0f );
}
