#include "ShaderCommon.hlsl"
#include "VertexFontCommon.hlsl"


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

Texture2D <float4> tNoise : register( t8 );

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
	float4 noise = tNoise.Sample( sSampler, input.uv + float2( 0.f, SYSTEM_TIME_SECONDS * .1f ) );

	float threshold = .7f;

	float noiseFactor = noise.x;

	clip( color.a * threshold - noiseFactor );

	float proportionIntoText = ( input.charIndex + 1 ) / input.specialEffects.x;
	float sinTime = ( 1.f + sin( SYSTEM_TIME_SECONDS ) ) * .6f;

	clip( sinTime - proportionIntoText );
	//clip( ( 1.f + sin( SYSTEM_TIME_SECONDS * .5f ) ) * input.specialEffects.x - input.charIndex );

	float4 outputColor = color * input.color;

	outputColor.r = sin( SYSTEM_TIME_SECONDS * .25f );
	outputColor.b = cos( SYSTEM_TIME_SECONDS * .5f );
	outputColor.a = 1.f;
	return outputColor;
}
