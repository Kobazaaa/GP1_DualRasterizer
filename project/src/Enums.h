#pragma once

enum class SamplerState
{
	Point,
	Linear,
	Anisotropic
};
enum class ShadingMode
{
	ObservedArea,	// Lambert Cosine Law
	Diffuse,		// Diffuse Color
	Specular,		// Specular Color
	Combined		// Diffuse + Specular + Ambient
};
enum class CullMode
{
	BackFace,
	FrontFace,
	None,
};
