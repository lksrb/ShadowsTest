#pragma once

struct quad_vertex
{
	v4 Position;
	v4 Color;
	v3 Normal;
};

struct quad_root_signature_constant_buffer
{
	m4 ViewProjection;
	m4 View;
	m4 LightSpaceMatrix;
};

struct shadow_pass_root_signature_constant_buffer
{
	m4 LightSpaceMatrix;
};

struct camera
{
	m4 Projection{ 1.0f };
	m4 View{ 1.0f };

	f32 OrthographicSize = 15.0f;
	f32 OrthographicNear = -100.0f, OrthographicFar = 100.0f;

	f32 AspectRatio = 0.0f;

	f32 PerspectiveFOV = glm::pi<f32>() / 2;// bkm::PI_HALF;
	f32 PerspectiveNear = 0.1f, PerspectiveFar = 1000.0f;

	void RecalculateProjectionOrtho(u32 Width, u32 Height)
	{
		AspectRatio = static_cast<f32>(Width) / Height;
		f32 OrthoLeft = -0.5f * AspectRatio * OrthographicSize;
		f32 OrthoRight = 0.5f * AspectRatio * OrthographicSize;
		f32 OrthoBottom = -0.5f * OrthographicSize;
		f32 OrthoTop = 0.5f * OrthographicSize;
		Projection = glm::orthoLH_ZO(OrthoLeft, OrthoRight, OrthoBottom, OrthoTop, OrthographicNear, OrthographicFar);
	}

	void RecalculateProjectionPerspective(u32 Width, u32 Height)
	{
		AspectRatio = static_cast<f32>(Width) / Height;
		Projection = glm::perspectiveLH_ZO(PerspectiveFOV, AspectRatio, PerspectiveNear, PerspectiveFar);
	}

	m4 GetViewProjection() const { return Projection * View; }
};

struct point_light
{
	v3 Position;
	f32 Intensity;

	v3 Radiance;
	f32 Radius;

	f32 FallOff;
	v3 _Pad0;
};

struct directional_light
{
	v3 Direction;
	f32 Intensity;
	v3 Radiance;
	f32 _Pad0;
};

struct light_environment
{
	static constexpr u32 MaxPointLights = 64;
	static constexpr u32 MaxDirectionalLights = 4;

	directional_light DirectionalLight[MaxDirectionalLights];
	point_light PointLights[MaxPointLights];
	i32 PointLightCount = 0;
	i32 DirectionalLightCount = 0;

	inline void Clear() { DirectionalLightCount = PointLightCount = 0; };
	inline auto& EmplaceDirectionalLight() { Assert(DirectionalLightCount < MaxDirectionalLights, "Too many directional lights!"); return DirectionalLight[DirectionalLightCount++]; }
	inline auto& EmplacePointLight() { Assert(PointLightCount < MaxPointLights, "Too many point lights!"); return PointLights[PointLightCount++]; }
};
