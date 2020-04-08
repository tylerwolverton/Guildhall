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
	// use the uv to sample the texture
	float4 texture_color = tDiffuse.Sample( sSampler, input.uv );
	float3 surface_color = ( input.color.xyz * pow( max( texture_color.xyz, 0.f ), 1.f / GAMMA ) ); // multiply our tint with our texture color to get our final color; 
	float surface_alpha = ( input.color.a * texture_color.a );

	float3 ambient = AMBIENT.xyz * AMBIENT.w;

	float4 normal_color = tNormals.Sample( sSampler, input.uv );
	float3 surface_normal = ColorToSurfaceColor( normal_color.xyz ); // (0 to 1) space to (-1, -1, 0),(1, 1, 1) space

	float3 surface_tangent = normalize( input.world_tangent );
	float3 surface_bitangent = normalize( input.world_bitangent );

	float3x3 tbn = float3x3( surface_tangent, surface_bitangent, normalize( input.world_normal ) );
	surface_normal = mul( surface_normal, tbn );
	
	// for each light, we going to add in dot3 factor it
	float3 light_position = LIGHT.world_position;
	float3 dir_to_light = normalize( light_position - input.world_position );
	float dot3 = max( 0.0f, dot( dir_to_light, surface_normal ) * LIGHT.intensity );

	float a = LIGHT.attenuation.x;
	float b = LIGHT.attenuation.y;
	float c = LIGHT.attenuation.z;
	float d = distance( input.world_position, light_position );

	float attenuation = 1.f / ( a + (b*d) + (c*d*d) );

	float3 diffuse = dot3 * LIGHT.color * attenuation;

	// just diffuse lighting
	diffuse = min( float3( 1,1,1 ), diffuse );
	diffuse = saturate( diffuse ); // saturate is clamp01(v)

	// specular
	float3 viewDir = normalize( CAMERA_WORLD_POSITION - input.world_position );
	float3 reflectDir = normalize( reflect( -dir_to_light, surface_normal ) );

	float spec = pow( max( dot( viewDir, reflectDir ), 0.0f ), SPECULAR_POWER );

	a = LIGHT.specular_attenuation.x;
	b = LIGHT.specular_attenuation.y;
	c = LIGHT.specular_attenuation.z;

	float specular_attenuation = 1.f / ( a + ( b * d ) + ( c * d * d ) );

	float3 specular = SPECULAR_FACTOR * spec * specular_attenuation;

	float3 final_color = ( ambient + diffuse + specular ) * surface_color;
	final_color = pow( max( final_color, 0.f ), GAMMA );

	return float4( final_color, surface_alpha );
}
