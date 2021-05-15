//--------------------------------------------------------------------------------------
// Stream Input
//--------------------------------------------------------------------------------------

struct vs_input_t
{
	// we are not defining our own input data; 
	float3 position       : POSITION;
	float4 color          : COLOR;
	float2 uv             : TEXCOORD;
	float2 glyphPos       : GLYPHPOSITION;
	float2 textPos        : TEXTPOSITION;
	int    charIndex      : CHARINDEX;
	float4 specialEffects : SPECIALEFFECTS;
};


//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t
{
	float4 position       : SV_POSITION;
	float4 color          : COLOR;
	float2 uv 			  : UV;
	float2 glyphPos       : GLYPHPOS;
	float2 textPos        : TEXTPOS;
	int    charIndex      : CHARINDEX;
	float4 specialEffects : SPECIALEFFECTS;
};
