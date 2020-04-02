#include "Engine/Renderer/BuiltInShaders.hpp"


//--------------------------------------------------------------------------------------
const char* g_defaultShaderCode = R"(
struct vs_input_t
{
	// we are not defining our own input data; 
	float3 position      : POSITION;
	float4 color         : COLOR;
	float2 uv            : TEXCOORD;
};


cbuffer time_constants : register( b0 )
{
	float SYSTEM_TIME_SECONDS;
	float SYSTEM_TIME_DELTA_SECONDS;
};


// MVP - Model - View - Projection
cbuffer camera_constants : register( b1 )
{
	float4x4 VIEW;		 // WORLD_TO_CAMERA_TRANSFORM;
	float4x4 PROJECTION; // CAMERA_TO_CLIP_TRANSFORM;  
};


cbuffer model_matrix_constants : register( b2 )
{
	float4x4 MODEL;
	float4 TINT;
};


// data - uniform/constant across entire draw call
Texture2D<float4> tDiffuse : register( t0 );	// color of surface
SamplerState sSampler : register( s0 );			// rules for how to sample texture


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : UV;
};


//--------------------------------------------------------------------------------------
// Default Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	v2f_t v2f = (v2f_t)0;

	// forward vertex input onto the next stage
	v2f.position = float4( input.position, 1.0f );
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;

	float4 worldPos = float4( input.position, 1 );
	float4 modelPos = mul( MODEL, worldPos );
	float4 cameraPos = mul( VIEW, worldPos );
	float4 clipPos = mul( PROJECTION, cameraPos );

	v2f.position = clipPos;

	return v2f;
}


//--------------------------------------------------------------------------------------
// Default Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float4 color = tDiffuse.Sample( sSampler, input.uv );
	return color * input.color;
}
)";


//--------------------------------------------------------------------------------------
const char* g_errorShaderCode = R"(
struct vs_input_t
{
	// we are not defining our own input data; 
	float3 position      : POSITION;
	float4 color         : COLOR;
	float2 uv            : TEXCOORD;
};


cbuffer time_constants : register( b0 )
{
	float SYSTEM_TIME_SECONDS;
	float SYSTEM_TIME_DELTA_SECONDS;
};


// MVP - Model - View - Projection
cbuffer camera_constants : register( b1 )
{
	float4x4 VIEW;		 // WORLD_TO_CAMERA_TRANSFORM;
	float4x4 PROJECTION; // CAMERA_TO_CLIP_TRANSFORM;  
};


// data - uniform/constant across entire draw call
Texture2D<float4> tDiffuse : register( t0 );	// color of surface
SamplerState sSampler : register( s0 );			// rules for how to sample texture


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : UV;
};


//--------------------------------------------------------------------------------------
// Default Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	v2f_t v2f = (v2f_t)0;

	// forward vertex input onto the next stage
	v2f.position = float4( input.position, 1.0f );
	v2f.color = input.color;
	v2f.uv = input.uv;

	float4 worldPos = float4( input.position, 1 );
	float4 cameraPos = mul( VIEW, worldPos );
	float4 clipPos = mul( PROJECTION, cameraPos );

	v2f.position = clipPos;

	return v2f;
}


//--------------------------------------------------------------------------------------
// Default Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float4 color;
	color.r = 1.f;
	color.g = 0.f;
	color.b = 1.f;
	color.a = 1.f;

	return color;
})";
