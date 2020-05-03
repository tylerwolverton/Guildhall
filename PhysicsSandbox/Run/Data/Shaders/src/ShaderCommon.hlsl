
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
	float4   TINT;
};


cbuffer material_constants : register( b3 )
{
	float4   START_TINT;
	float4   END_TINT;
	float    TINT_RATIO;
};
