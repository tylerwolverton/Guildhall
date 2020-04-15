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

	return light_intensity / ( constant_att_factor
							   + ( linear_att_factor * dist )
							   + ( quadratic_att_factor * dist * dist ) );
}


//--------------------------------------------------------------------------------------
float3 CalculateDot3Light( float3 world_position, float3 world_normal, float3 surface_color )
{
	float3 ambient = AMBIENT.xyz * AMBIENT.w;
	// for each light, we going to add the dot3 and specular factors
	float3 diffuse = float3( 0.f, 0.f, 0.f );
	float3 specular = float3( 0.f, 0.f, 0.f );

	for ( int i = 0; i < MAX_NUM_LIGHTS; ++i )
	{
		float3 light_position = LIGHTS[i].world_position;
		float3 incident_dir = normalize( lerp( world_position - light_position,
											   LIGHTS[i].direction,
											   LIGHTS[i].is_directional ) );

		float dot_incident = dot( -incident_dir, world_normal );
		float dot3 = max( 0.0f, dot_incident );

		float dist_to_light = distance( world_position, light_position );
		float dist = lerp( dist_to_light,
						   dot( dist_to_light, LIGHTS[i].direction ),
						   LIGHTS[i].is_directional );


		float dot_angle = dot( LIGHTS[i].direction, normalize( world_position - light_position ) );
		float val = saturate( RangeMap( dot_angle, LIGHTS[i].half_cos_outer_angle, LIGHTS[i].half_cos_inner_angle, 0.f, 1.f ) );

		float attenuation = CalculateAttenuation( LIGHTS[i].attenuation, LIGHTS[i].intensity, dist );
		attenuation *= val;

		diffuse += dot3 * attenuation * LIGHTS[i].color;

		// specular
		float3 view_dir = normalize( CAMERA_WORLD_POSITION - world_position );
		float3 half_dir = normalize( -incident_dir + view_dir );
		float facing_factor = smoothstep( -.3f, 0.1f, dot_incident );

		float spec = pow( max( dot( world_normal, half_dir ), 0.0f ), SPECULAR_POWER );

		float specular_attenuation = CalculateAttenuation( LIGHTS[i].specular_attenuation, LIGHTS[i].intensity, dist );
		specular_attenuation *= val;

		specular += SPECULAR_FACTOR * spec * specular_attenuation * facing_factor * LIGHTS[i].color;
	}

	diffuse = min( diffuse, float3( 1.f, 1.f, 1.f ) );

	return ( ambient + diffuse ) * surface_color + specular;
}