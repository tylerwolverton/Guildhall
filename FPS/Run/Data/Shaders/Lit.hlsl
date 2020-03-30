#include "ShaderUtils.hlsl"


//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	return DefaultVertexFunction( input );
}


struct light_t
{
	float3 world_position;
	float pad00; // not required but GPU will add it

	float3 color;
	float intensity; 
};


cbuffer light_constants : register( b3 )
{
	float4 AMBIENT;
};


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float4 texture_color = tDiffuse.Sample( sSampler, input.uv );
	float3 surface_color = ( input.color * texture_color ).xyz;
	float3 surface_alphs = ( input.color.a * texture_color.a );

	float3 diffuse = AMBIENT.xyz * AMBIENT.w;

	float3 surface_normal = normalize( input.world_normal );

	// for each light, we're going to add in dot3 factor it
	float3 light_position = LIGHT.world_position;
	float3 dir_to_light = normalize( light_position - input.world_position );
	float dot3 = max(0.0f, dot( dir_to_light, surface_normal ) );

	diffuse += dot3;

	diffuse = min( float3( 1, 1, 1 ), diffuse );
	diffuse = saturate( diffuse );
	float final_color = diffuse * surface_color;

	return float4( final_color, surface_alpha );
}
