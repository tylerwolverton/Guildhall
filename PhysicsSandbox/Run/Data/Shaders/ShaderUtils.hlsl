float RangeMap( float val, float inMin, float inMax, float outMin, float outMax )
{
	float inputRange = inMax - inMin;
	float outputRange = outMax - outMin;
	return ( ( val - inMin ) / inputRange ) * outputRange + outMin;
}
