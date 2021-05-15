#include "ShaderCommon.hlsl"
#include "PCUCommon.hlsl"


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

	float letterThickness = .6f;
	float softness = .15f;
	float outerThreshhold = letterThickness;
	float innerThreshhold = letterThickness + softness;

	clip( color.a - outerThreshhold );

	float alpha = ( color.a - outerThreshhold ) / ( innerThreshhold - outerThreshhold );

	float4 outputColor = color * input.color;
	outputColor.a = alpha;
	return outputColor;
}
