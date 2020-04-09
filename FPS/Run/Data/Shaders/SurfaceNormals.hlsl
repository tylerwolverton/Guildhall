#include "ShaderCommon.hlsl"
#include "ShaderUtils.hlsl"
#include "PCUTBN_Common.hlsl"


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

	// tangent
	float4 localTangent = float4( input.tangent, 0.0f );
	float4 worldTangent = mul( MODEL, localTangent );
	
	// bitangent
	float4 localBitangent = float4( input.bitangent, 0.0f );
	float4 worldBitangent = mul( MODEL, localBitangent );

	// normal is currently in model/local space
	float4 localNormal = float4( input.normal, 0.0f );
	float4 worldNormal = mul( MODEL, localNormal );

	v2f.position = clipPos; // we want to output the clip position to raster (a perspective point)
	v2f.color = input.color * TINT;
	v2f.uv = input.uv;

	v2f.world_position = worldPos.xyz;
	v2f.world_tangent = worldTangent.xyz;
	v2f.world_bitangent = worldBitangent.xyz;
	v2f.world_normal = worldNormal.xyz;

	return v2f;
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float4 texture_color = tNormals.Sample( sSampler, input.uv ); 
	float3 surface_normal = ColorToVector( texture_color.xyz ); // (0 to 1) space to (-1, -1, 0),(1, 1, 1) space

	float3 surface_tangent = normalize( input.world_tangent );
	float3 surface_bitangent = normalize( input.world_bitangent );

	float3x3 tbn = float3x3( surface_tangent, surface_bitangent, normalize( input.world_normal ) );
	surface_normal = mul( surface_normal, tbn );

	return float4( VectorToColor( surface_normal ), 1.f );
}
