#include "ShaderCommon.hlsl"
#include "PCUTBN_Common.hlsl"


// Textures & Samplers are also a form of constant
// data - uniform/constant across the entire call
Texture2D <float4> tDiffuse   : register( t0 );   // color of the surface
SamplerState sSampler : register( s0 );           // sampler are rules on how to sample (read) from the texture.


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	v2f_t v2f = (v2f_t)0;

	// move the vertex through the spaces
	float4 worldPos = float4( input.position, 1.0f );
	float4 modelPos = mul( MODEL, worldPos );
	float4 cameraPos = mul( VIEW, modelPos );
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

	v2f.world_position = modelPos.xyz;
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
	// use the uv to sample the texture
	float4 texture_color = tDiffuse.Sample( sSampler, input.uv );
	float3 surface_color = ( input.color * texture_color ).xyz; // multiply our tint with our texture color to get our final color; 
	float surface_alpha = ( input.color.a * texture_color.a );

	float3 ambient = AMBIENT.xyz * AMBIENT.w; // ambient color * ambient intensity

	// get my surface normal - this comes from the vertex format
	// We now have a NEW vertex format
	float3 surface_normal = normalize( input.world_normal );

	// for each light, we going to add in dot3 factor it
	float3 light_position = LIGHT.world_position;
	float3 dir_to_light = normalize( light_position - input.world_position );
	float dot3 = max( 0.0f, dot( dir_to_light, surface_normal ) * LIGHT.intensity );

	float3 diffuse = dot3;// *LIGHT.color;

	// just diffuse lighting
	diffuse = min( float3( 1,1,1 ), diffuse );
	diffuse = saturate( diffuse ); // saturate is clamp01(v)

	float3 final_color = ( ambient + diffuse ) * surface_color;
	//final_color += LIGHT.color;

	return float4( final_color, surface_alpha );
}
