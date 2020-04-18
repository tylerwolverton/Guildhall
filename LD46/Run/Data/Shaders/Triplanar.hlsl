#include "ShaderCommon.hlsl"
#include "ShaderUtils.hlsl"
#include "LightingUtils.hlsl"
#include "PCUTBN_Common.hlsl"


Texture2D <float4> tDiffuse1 : register( t8 );
Texture2D <float4> tDiffuse2 : register( t9 );
Texture2D <float4> tDiffuse3 : register( t10 );
Texture2D <float4> tNormal1 : register( t11 );
Texture2D <float4> tNormal2 : register( t12 );
Texture2D <float4> tNormal3 : register( t13 );

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
	float2 x_uv = frac( input.world_position.zy );
	//x_uv.x = lerp( x_uv.x, 1.f - x_uv.x, saturate( sign( input.world_normal.x ) ) );
		
	float2 y_uv = frac( input.world_position.xz );
	float2 z_uv = frac( input.world_position.xy );

	float4 x_color = tDiffuse1.Sample( sSampler, x_uv );
	float4 y_color = tDiffuse2.Sample( sSampler, y_uv );
	float4 z_color = tDiffuse3.Sample( sSampler, z_uv );

	float3 x_normal = tNormal1.Sample( sSampler, x_uv ).xyz;
	float3 y_normal = tNormal2.Sample( sSampler, y_uv ).xyz;
	float3 z_normal = tNormal3.Sample( sSampler, z_uv ).xyz;

	float3 weights = normalize( input.world_normal );
	weights = abs( weights );

	float sum = weights.x + weights.y + weights.z;
	weights /= sum;

	float4 final_color =  weights.x * x_color
						+ weights.y * y_color 
						+ weights.z * z_color;

	x_normal = ColorToVector( x_normal );
	y_normal = ColorToVector( y_normal );
	z_normal = ColorToVector( z_normal );

	// Adjust normals to account for different sides of plane
	z_normal.z *= sign( input.world_normal.z );

	float3x3 x_tbn = float3x3( float3( 0, 0, -1 ),
							   float3( 0, 1, 0 ),
							   float3( 1, 0, 0 ) );


	x_normal = mul( x_normal, x_tbn );
	x_normal.x *= sign( input.world_normal.x );
	
	float3x3 y_tbn = float3x3( float3( 1, 0, 0 ),
							   float3( 0, 0, -1 ),
							   float3( 0, 1, 0 ) );

	y_normal = mul( y_normal, y_tbn );
	y_normal.y *= sign( input.world_normal.y );

	float3 final_normal = weights.x * x_normal
						+ weights.y * y_normal
						+ weights.z * z_normal;

	final_normal = normalize( final_normal );
	
	final_color = pow( max( final_color, 0.f ), GAMMA.xxxx );

	float3 surface_color = CalculateDot3Light( input.world_position, final_normal, final_color.xyz );

	surface_color = pow( max( surface_color, 0.f ), ( 1 / GAMMA ).xxx );

	float4 final_color_with_fog = AddFogToColor( float4( surface_color, 1.f ), input.world_position );
	return final_color_with_fog;
}
