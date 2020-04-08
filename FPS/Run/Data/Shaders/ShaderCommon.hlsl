
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
};


cbuffer material_constants : register( b3 )
{
	float4   START_TINT;
	float4   END_TINT;
	float    TINT_RATIO;

	float    SPECULAR_FACTOR;
	float    SPECULAR_POWER;
};


struct light_t
{
	float3 world_position;
	float pad00;  // this is not required, but know the GPU will add this padding to make the next variable 16-byte aligned

	float3 color;
	float intensity; // rgb and an intensity

	float3 attenuation;
	float pad01;

	float3 specular_attenuation;
	float pad02;
};


cbuffer light_constants : register( b4 )
{
	float4 AMBIENT;
	light_t LIGHT;
};

// Textures & Samplers are also a form of constant
// data - uniform/constant across the entire call
Texture2D <float4> tDiffuse   : register( t0 );   // color of the surface
Texture2D <float4> tNormals   : register( t1 );   // normal map of the surface
SamplerState sSampler : register( s0 );           // sampler are rules on how to sample (read) from the texture.
