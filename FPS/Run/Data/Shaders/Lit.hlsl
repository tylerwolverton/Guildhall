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
float CalculateAttenuation( float3 attenuation_factors, float light_intensity, 
							float3 pixel_world_position, float3 light_position )
{
	float constant_att_factor = attenuation_factors.x;
	float linear_att_factor = attenuation_factors.y;
	float quadratic_att_factor = attenuation_factors.z;
	float dist = distance( pixel_world_position, light_position );

	return light_intensity / ( constant_att_factor
							+ ( linear_att_factor * dist )
							+ ( quadratic_att_factor * dist * dist ) );
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
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

	float3 ambient = AMBIENT.xyz * AMBIENT.w;
	// for each light, we going to add the dot3 and specular factors
	float3 diffuse = float3(0.f, 0.f, 0.f);
	float3 specular = float3( 0.f, 0.f, 0.f );

	for ( int i = 0; i < MAX_NUM_LIGHTS; ++i )
	{
		float3 light_position = LIGHTS[i].world_position;
		float3 dir_to_light = normalize( light_position - input.world_position );

		float dot_incident = dot( dir_to_light, world_normal );
		float dot3 = max( 0.0f, dot_incident );
		
		float attenuation = CalculateAttenuation( LIGHTS[i].attenuation, LIGHTS[i].intensity,
												  input.world_position, light_position );

		diffuse += dot3 * attenuation * LIGHTS[i].color;

		// specular
		float3 view_dir = normalize( CAMERA_WORLD_POSITION - input.world_position );
		float3 half_dir = normalize( dir_to_light + view_dir );
		float facing_factor = smoothstep( -.3f, 0.1f, dot_incident );

		float spec = pow( max( dot( world_normal, half_dir ), 0.0f ), SPECULAR_POWER );

		float specular_attenuation = CalculateAttenuation( LIGHTS[i].specular_attenuation, LIGHTS[i].intensity,
														   input.world_position, light_position );

		specular += SPECULAR_FACTOR * spec * specular_attenuation * facing_factor * LIGHTS[i].color;
	}

	diffuse = min( diffuse, float3( 1.f, 1.f, 1.f ) );

	float3 final_color = ( ambient + diffuse ) * surface_color + specular;
	final_color = pow( max( final_color, 0.f ), 1.f / GAMMA );

	return float4( final_color, surface_alpha );
}