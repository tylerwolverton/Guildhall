#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Rgba8.hpp"


//-----------------------------------------------------------------------------------------------
enum class eFillMode : uint
{
	WIREFRAME,
	SOLID,
};


//-----------------------------------------------------------------------------------------------
enum class eBlendMode
{
	DISABLED,
	ALPHA,
	ADDITIVE,
};


//-----------------------------------------------------------------------------------------------
enum class eCullMode : uint
{
	NONE,
	FRONT,
	BACK,
};


//-----------------------------------------------------------------------------------------------
enum class eCompareFunc : uint
{
	COMPARISON_NEVER,
	COMPARISON_LESS,
	COMPARISON_LESS_EQUAL,
	COMPARISON_GREATER,
	COMPARISON_GREATER_EQUAL,
	COMPARISON_EQUAL,
	COMPARISON_ALWAYS,
};

//-----------------------------------------------------------------------------------------------
// Should this be controlled by 2 separate enums?
enum class eSampler
{
	POINT_CLAMP,
	LINEAR_CLAMP,
	POINT_WRAP,
};


//-----------------------------------------------------------------------------------------------
enum eBufferSlot
{
	UBO_FRAME_SLOT = 0,
	UBO_CAMERA_SLOT = 1,
	UBO_MODEL_MATRIX_SLOT = 2,
	UBO_LIGHT_SLOT = 3,
	UBO_MATERIAL_SLOT = 5,
	UBO_DEBUG_LIGHT_SLOT = 10,
};


//-----------------------------------------------------------------------------------------------
struct FrameData
{
	float systemTimeSeconds;
	float systemDeltaTimeSeconds;

	float nearFogDistance = 50.f;
	float farFogDistance = 100.f;

	Vec4 fogColor;

	float gamma = 2.2f;
	float padding0;
	float padding1;
	float padding2;
};


//-----------------------------------------------------------------------------------------------
struct ModelData
{
	Mat44 modelMatrix;
	float tint[4];

	float specularFactor;
	float specularPower;

	float padding0;
	float padding1;
};


//-----------------------------------------------------------------------------------------------
struct Light
{
	Vec3 position = Vec3::ONE;
	float pad00;

	Vec3 direction = Vec3( 0.f, 0.f, -1.f );
	float isDirectional = 0.f;

	Vec3 color = Vec3::ONE;
	float intensity = 0.f;

	Vec3 attenuation = Vec3( 0.f, 1.f, 0.f );
	float halfCosOfInnerAngle = -1.f;

	Vec3 specularAttenuation = Vec3( 0.f, 1.f, 0.f );
	float halfCosOfOuterAngle = -1.f;
};

struct LightData
{
	Vec4 ambientLight;
	Light lights[MAX_LIGHTS];
};

struct DebugLightData
{
	float diffuseEffect = 1.f;
	float specularEffect = 1.f;
	float emissiveEffect = 1.f;

	float padding0;
};


//-----------------------------------------------------------------------------------------------
struct Fog
{
	float nearFogDistance = 999999.f;
	float farFogDistance = 999999.f;

	Vec4 fogColor = Rgba8::BLACK.GetAsRGBAVector();
};
