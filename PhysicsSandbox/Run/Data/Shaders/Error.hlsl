#include "ShaderUtils.hlsl"


//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction( vs_input_t input )
{
	return DefaultVertexFunction( input );
}


//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// Draw as magenta to signal error
//
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float4 color;
	color.r = 1.f;
	color.g = 0.f;
	color.b = 1.f;
	color.a = 1.f;

	return color;
}
