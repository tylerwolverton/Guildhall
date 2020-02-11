//--------------------------------------------------------------------------------------
// Stream Input
// ------
// Stream Input is input that is walked by the vertex shader.  
// If you say "Draw(3,0)", you are telling to the GPU to expect '3' sets, or 
// elements, of input data.  IE, 3 vertices.  Each call of the VertxShader
// we be processing a different element. 
//--------------------------------------------------------------------------------------

// inputs are made up of internal names (ie: uv) and semantic names
// (ie: TEXCOORD).  "uv" would be used in the shader file, where
// "TEXCOORD" is used from the client-side (cpp code) to attach ot. 
// The semantic and internal names can be whatever you want, 
// but know that semantics starting with SV_* usually denote special 
// inputs/outputs, so probably best to avoid that naming.
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


cbuffer camera_constants : register( b1 )
{
	float2 orthoMin;
	float2 orthoMax;
};


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

// TODO: Move to MathUtils.hlsl
float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
	float inputRange = inMax - inMin;
	float outputRange = outMax - outMin;
	return ( ( val - inMin ) / inputRange ) * outputRange + outMin;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	v2f_t v2f = (v2f_t)0;

	// forward vertex input onto the next stage
	v2f.position = float4( input.position, 1.0f );
	v2f.color = input.color;
	v2f.uv = input.uv;

	float4 worldPos = float4( input.position, 1 );
	//worldPos.x += cos( SYSTEM_TIME_SECONDS );
	//worldPos.y += sin( SYSTEM_TIME_SECONDS );

	float4 clipPos = worldPos;
	clipPos.x = RangeMap( worldPos.x, orthoMin.x, orthoMax.x, -1.f, 1.f );
	clipPos.y = RangeMap( worldPos.y, orthoMin.y, orthoMax.y, -1.f, 1.f );
	clipPos.z = 0.f;
	clipPos.w = 1.f;

	//clipPos.w = .75f + sin( sin( SYSTEM_TIME_SECONDS * 2.f) * cos( SYSTEM_TIME_SECONDS * 5.f ) );
	//clipPos.xyz /= clipPos.w;

	v2f.position = clipPos;

	return v2f;
}

// raster step
// float3 ndcPos = clipPos.xyz / clipPos.w;

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	// we'll outoupt our UV coordinates as color here
	// to make sure they're being passed correctly.
	// Very common rendering debugging method is to 
	// use color to portray information; 
	float4 uvAsColor = float4( input.uv, 0.0f, 1.0f );
	float4 finalColor = input.color;;

	/*finalColor.r = 1.f + input.uv.x + sin( SYSTEM_TIME_SECONDS );
	finalColor.g = 1.f + input.uv.x + cos( SYSTEM_TIME_SECONDS );
	finalColor.b = finalColor.r - finalColor.g;*/

	return finalColor;
}
