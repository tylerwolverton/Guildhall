float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
	float inputRange = inMax - inMin;
	float outputRange = outMax - outMin;
	return ( ( val - inMin ) / inputRange ) * outputRange + outMin;
}


float3 ColorToSurfaceColor( float3 color )
{
	return float3( ( color.x * 2.f ) - 1.f, ( color.y * 2.f ) - 1.f, color.z );
}


float3 SurfaceColorToColor( float3 surface_color )
{
	return float3( ( surface_color.x + 1.f ) * .5f, ( surface_color.y + 1.f ) * .5f, surface_color.z );
}
