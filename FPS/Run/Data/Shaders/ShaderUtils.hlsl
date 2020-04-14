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