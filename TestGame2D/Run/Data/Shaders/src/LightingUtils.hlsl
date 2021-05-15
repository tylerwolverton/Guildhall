//--------------------------------------------------------------------------------------
struct light_t
{
	float3 world_position;
	float pad00;  // this is not required, but know the GPU will add this padding to make the next variable 16-byte aligned

	float3 direction;
	float is_directional;

	float3 color;
	float intensity;

	float3 attenuation;
	float half_cos_inner_angle;

	float3 specular_attenuation;
	float half_cos_outer_angle;
};


//--------------------------------------------------------------------------------------
struct lit_color_t
{
	float3 color;
	float padding0;
	
	float3 bloom;
	float padding1;
};


//--------------------------------------------------------------------------------------
cbuffer light_constants : register( b3 )
{
	float4 AMBIENT;
	light_t LIGHTS[8];
};


//--------------------------------------------------------------------------------------
float CalculateAttenuation( float3 attenuation_factors, float light_intensity, float dist )
{
	float constant_att_factor = attenuation_factors.x;
	float linear_att_factor = attenuation_factors.y;
	float quadratic_att_factor = attenuation_factors.z;

	float attentuation = 1.f / ( constant_att_factor
							   + ( linear_att_factor * dist )
							   + ( quadratic_att_factor * dist * dist ) );

	return saturate( attentuation );
}


//--------------------------------------------------------------------------------------
lit_color_t CalculateDot3Light( float3 world_position, float3 world_normal, float3 surface_color, float4 specGlossEmit_color )
{
	float3 ambient = AMBIENT.xyz * AMBIENT.w;
	// for each light, we going to add the dot3 and specular factors
	float3 diffuse = float3( 0.f, 0.f, 0.f );
	float3 specular = float3( 0.f, 0.f, 0.f );

	float specular_factor_from_texture = specGlossEmit_color.r;
	float specular_power = RangeMap( specGlossEmit_color.g, 0.f, 1.f, 0.f, 32.f );

	for ( int i = 0; i < MAX_NUM_LIGHTS; ++i )
	{
		float3 light_position = LIGHTS[i].world_position;
		float3 incident_dir = normalize( lerp( world_position - light_position,
											   LIGHTS[i].direction,
											   LIGHTS[i].is_directional ) );

		float dot_incident = dot( -incident_dir, world_normal );
		float dot3 = max( 0.0f, dot_incident );

		float dist_to_light = distance( world_position, light_position );
		float3 dir_to_light = normalize( light_position - world_position );
		
		float dist = lerp( dist_to_light,
						   dot( -dir_to_light, LIGHTS[i].direction ),
						   LIGHTS[i].is_directional );


		float dot_angle = dot( LIGHTS[i].direction, normalize( world_position - light_position ) );
		float spotlight_attenuation = saturate( RangeMap( dot_angle, LIGHTS[i].half_cos_outer_angle, LIGHTS[i].half_cos_inner_angle, 0.f, 1.f ) );

		float attenuation = CalculateAttenuation( LIGHTS[i].attenuation, LIGHTS[i].intensity, dist );
		attenuation *= spotlight_attenuation;

		diffuse += dot3 * attenuation * LIGHTS[i].intensity * LIGHTS[i].color;

		// specular
		float3 view_dir = normalize( CAMERA_WORLD_POSITION - world_position );
		float3 half_dir = normalize( -incident_dir + view_dir );
		float facing_factor = max( smoothstep( -.3f, 0.1f, dot_incident ), 0.f );

		float specular_factor = specular_factor_from_texture * pow( max( dot( world_normal, half_dir ), 0.0f ), specular_power );

		float specular_attenuation = CalculateAttenuation( LIGHTS[i].specular_attenuation, LIGHTS[i].intensity, dist );
		specular_attenuation *= spotlight_attenuation;

		specular += specular_factor * specular_attenuation * facing_factor * LIGHTS[i].intensity * LIGHTS[i].color * SPECULAR_EFFECT;
	}

	diffuse = saturate( diffuse ) * DIFFUSE_EFFECT;

	float3 emissive = surface_color * specGlossEmit_color.b * EMISSIVE_EFFECT;

	float3 bloom = max( float3( 0.f, 0.f, 0.f ), ( specular + emissive ) - float3( 1.f, 1.f, 1.f ) );
	float3 color = ( ambient + diffuse ) * surface_color + specular + emissive;

	lit_color_t lit_color;
	lit_color.color = color; 
	lit_color.bloom = bloom;
	return lit_color;
}


//--------------------------------------------------------------------------------------
float4 AddFogToColor( float4 color, float3 world_position )
{
	float dist_from_camera = distance( CAMERA_WORLD_POSITION, world_position );

	float fog_factor = saturate( RangeMap( dist_from_camera, FOG_NEAR_DISTANCE, FOG_FAR_DISTANCE, 0.f, 1.f ) );

	return lerp( color, FOG_COLOR, fog_factor );
}