//--------------------------------------------------------------------------------------
// Stream Input
//--------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 position      : POSITION;
	float4 color         : COLOR;
	float2 uv            : TEXCOORD;

	float3 tangent       : TANGENT;
	float3 bitangent     : BITANGENT;
	float3 normal        : NORMAL;
};


//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv : UV;

	float3 world_position : WORLD_POSITION;
	float3 world_tangent : WORLD_TANGENT;
	float3 world_bitangent : WORLD_BITANGENT;
	float3 world_normal : WORLD_NORMAL;
};
