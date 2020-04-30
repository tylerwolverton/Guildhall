#include "ShaderCommon.hlsl"
#include "ShaderUtils.hlsl"
#include "LightingUtils.hlsl"
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

struct fragment_output_t
{
	float4 color : SV_TARGET0;
	float4 bloom : SV_TARGET1;
	float4 normal : SV_TARGET2;
	float4 albedo : SV_TARGET3;
	float4 tangent : SV_TARGET4;
};

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
fragment_output_t FragmentFunction( v2f_t input )
{
	// use the uv to sample the texture
	float4 diffuse_color = tDiffuse.Sample( sSampler, input.uv );
	float4 normal_color = tNormals.Sample( sSampler, input.uv );

	float3 surface_color = input.color.xyz * pow( max( diffuse_color.xyz, 0.f ), GAMMA ); // multiply our tint with our texture color to get our final color; 
	float surface_alpha = input.color.a * diffuse_color.a;

	float3x3 tbn = float3x3( normalize( input.world_tangent ), 
							 normalize( input.world_bitangent ), 
							 normalize( input.world_normal ) );

	float3 surface_normal = ColorToVector( normal_color.xyz ); // (0 to 1) space to (-1, -1, 0),(1, 1, 1) space
	float3 world_normal = mul( surface_normal, tbn );

	float3 final_color = CalculateDot3Light( input.world_position, world_normal, surface_color );
	
	final_color = pow( max( final_color, 0.f ), 1.f / GAMMA );

	float4 final_color_with_fog = AddFogToColor( float4( final_color, surface_alpha ), input.world_position );

	fragment_output_t output;
	output.color = final_color_with_fog;
	output.bloom = float4( 0, 0, 0, 1 );
	output.normal = float4( VectorToColor( world_normal ), 1.f);
	output.albedo = diffuse_color;
	output.tangent = float4( VectorToColor( input.world_tangent ), 1.f );

	return output;
}