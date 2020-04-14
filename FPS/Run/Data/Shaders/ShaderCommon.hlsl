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


cbuffer model_matrix_constants : register( b2 )
{
	float4x4 MODEL;
	float4   TINT;

	// Move to model constants
	float    SPECULAR_FACTOR;
	float    SPECULAR_POWER;
};


struct light_t
{
	float3 world_position;
	float pad00;  // this is not required, but know the GPU will add this padding to make the next variable 16-byte aligned

	float3 direction;
	float is_directional;

	float3 color;
	float intensity; 

	float3 attenuation;
	float half_cos_inner_angle;

	float3 specular_attenuation;
	float half_cos_outer_angle;
};


cbuffer light_constants : register( b3 )
{
	float4 AMBIENT;
	light_t LIGHTS[8];
};


// Textures & Samplers are also a form of constant
// data - uniform/constant across the entire call
Texture2D <float4> tDiffuse   : register( t0 );   // color of the surface
Texture2D <float4> tNormals   : register( t1 );   // normal map of the surface
SamplerState sSampler : register( s0 );           // sampler are rules on how to sample (read) from the texture.