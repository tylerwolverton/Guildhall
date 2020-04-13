//--------------------------------------------------------------------------------------
float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
	float inputRange = inMax - inMin;
	float outputRange = outMax - outMin;
	return ( ( val - inMin ) / inputRange ) * outputRange + outMin;
}


//--------------------------------------------------------------------------------------
float3 ColorToVector( float3 color )
{
	return float3( ( color.x * 2.f ) - 1.f, ( color.y * 2.f ) - 1.f, color.z );
}


//--------------------------------------------------------------------------------------
float3 VectorToColor( float3 vec )
{
	return float3( ( vec.x + 1.f ) * .5f, ( vec.y + 1.f ) * .5f, vec.z );
}


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