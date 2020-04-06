#include "ShaderCommon.hlsl"
#include "PCUCommon.hlsl"


// data - uniform/constant across entire draw call
Texture2D<float4> tDiffuse : register( t0 );	// color of surface
SamplerState sSampler : register( s0 );			// rules for how to sample texture


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	v2f_t v2f = (v2f_t)0;

	// move the vertex through the spaces
	float4 worldPos = float4( input.position, 1.0f );
	float4 modelPos = mul( MODEL, worldPos );
	float4 cameraPos = mul( VIEW, modelPos );
	float4 clipPos = mul( PROJECTION, cameraPos );

	v2f.position = clipPos;
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;

	return v2f;
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float4 color = tDiffuse.Sample( sSampler, input.uv );
	return color * input.color;
}
