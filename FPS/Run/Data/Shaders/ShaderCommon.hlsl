static const int MAX_NUM_LIGHTS = 8;


cbuffer time_constants : register( b0 )
{
	float SYSTEM_TIME_SECONDS;
	float SYSTEM_TIME_DELTA_SECONDS;

	float GAMMA;
};


// MVP - Model - View - Projection
cbuffer camera_constants : register( b1 )
{
	float4x4 VIEW;		 // WORLD_TO_CAMERA_TRANSFORM;
	float4x4 PROJECTION; // CAMERA_TO_CLIP_TRANSFORM;  
	float3 CAMERA_WORLD_POSITION;
};


cbuffer model_constants : register( b2 )
{
	float4x4 MODEL;
	float4   TINT;

	float    SPECULAR_FACTOR;
	float    SPECULAR_POWER;
};


// Textures & Samplers are also a form of constant
// data - uniform/constant across the entire call
Texture2D <float4> tDiffuse   : register( t0 );   // color of the surface
Texture2D <float4> tNormals   : register( t1 );   // normal map of the surface
SamplerState sSampler : register( s0 );           // sampler are rules on how to sample (read) from the texture.