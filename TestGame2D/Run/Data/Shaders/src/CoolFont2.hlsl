#include "ShaderCommon.hlsl"
#include "VertexFontCommon.hlsl"


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
	// v2f.position.y += input.textPos.x * sin( SYSTEM_TIME_SECONDS );
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;
	v2f.glyphPos = input.glyphPos;
	v2f.textPos = input.textPos;
	v2f.charIndex = input.charIndex;
	v2f.specialEffects = input.specialEffects;

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

	float threshold = .6f;
	float distanceFromCursorSquared = ( input.position.x - input.specialEffects.x ) * ( input.position.x - input.specialEffects.x ) 
									+ ( input.position.y -  input.specialEffects.y ) * ( input.position.y -  input.specialEffects.y );
	float distanceSquaredThreshold = 2000.f;

	clip( color.a - threshold );
	clip( distanceSquaredThreshold - distanceFromCursorSquared );

	float4 outputColor = color * input.color;
	outputColor.a = 1.f;
	return outputColor;
}
