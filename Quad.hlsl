#include "Light.hlsl"

cbuffer root_constants : register(b0)
{
    float4x4 c_ViewProjection;
    float4x4 c_ViewMatrix;
    float4x4 c_LightSpaceMatrix;
};

struct vertex_shader_input
{
    float4 Position : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
};

struct pixel_shader_input
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLDPOSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3 ViewPosition : VIEWPOSITION;
    float4 PositionInLightSpace : POSITIONINLIGHTSPACE;
};

pixel_shader_input VSMain(vertex_shader_input In)
{
    pixel_shader_input Out;

    Out.Position = mul(c_ViewProjection, In.Position);
    Out.WorldPosition = In.Position;
    Out.Color = In.Color;
    Out.Normal = In.Normal;
    Out.ViewPosition = mul(c_ViewMatrix, Out.WorldPosition).xyz;
    Out.PositionInLightSpace = mul(c_LightSpaceMatrix, float4(Out.WorldPosition)); // SHADOWS
    
    return Out;
}

// TODO: Reduce the amount of active point lights by calculating which light is visible and which is not
cbuffer light_environment : register(b1)
{
    directional_light u_DirectionalLights[4];
    point_light u_PointLights[64];
    int u_PointLightCount;
    int u_DirectionalLightCount;
};

Texture2D<float> g_ShadowMap : register(t0);
SamplerState g_ShadowMapSampler : register(s0);

float ShadowCalculation(float4 ShadowPos, directional_light Light, float3 Normal)
{
    float Shadow = 0.0f;
    
    // perform perspective divide
    ShadowPos.xyz /= ShadowPos.w;
    
    // transform to [0,1] range
    ShadowPos = ShadowPos * 0.5 + 0.5;
    
    // get depth of current fragment from light's perspective
    float CurrentDepth = ShadowPos.z;
    
    if (CurrentDepth > 1.0)
        return 0.0;
    
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float ClosestDepth = g_ShadowMap.Sample(g_ShadowMapSampler, ShadowPos.xy);
    
    //float Bias = 0.005;
    float3 LightDir = normalize(-Light.Direction);
    float Bias = max(0.05 * (1.0 - dot(Normal, LightDir)), 0.005);
    //float Bias = 0.02;
    Shadow = CurrentDepth > ClosestDepth ? 1.0 : 0.0;
    
    // check whether current frag pos is in shadow
    //Shadow = CurrentDepth > ClosestDepth ? 1.0 : 0.0;
    
    return Shadow;
}

float3 CalculateDirectionalLight2(directional_light Light, float3 Normal, float3 ViewDir, float Shininess, float3 TextureColor, float Shadow)
{
    float3 Result = float3(0.0, 0.0, 0.0);

    // Calculate direction of the Light source
    float3 LightDir = normalize(-Light.Direction);

    // Calculate diffuse 
    float DiffuseAngle = max(dot(Normal, LightDir), 0.0);

    // Calculate specular 
    float3 ReflectDir = reflect(-LightDir, Normal);
    float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess);

    // TODO: Materials
    float3 LightAmbient = float3(0.5, 0.5, 0.5);
    float3 LightDiffuse = float3(0.8, 0.8, 0.8);
    float3 LightSpecular = float3(1.0, 1.0, 1.0);

    // Combine results
    float3 Ambient = Light.Intensity * LightAmbient * Light.Radiance * TextureColor;
    float3 Diffuse = Light.Intensity * LightDiffuse * Light.Radiance * DiffuseAngle * TextureColor;
    //float3 Specular = Light.Intensity * LightSpecular * Light.Radiance * Spec * TextureColor;
    float3 Specular = float3(0.0, 0.0, 0.0);
    Result += (Ambient + (1.0 - Shadow) * (Diffuse + Specular));

    return Result;
}

float4 PSMain(pixel_shader_input In) : SV_TARGET
{
    float3 Normal = normalize(In.Normal);
    float3 ViewDir = normalize(In.ViewPosition - In.WorldPosition.xyz);
    float Shininess = 32.0;
    float ShadowValue = ShadowCalculation(In.PositionInLightSpace, u_DirectionalLights[0], Normal);
    
    // Phase 1: Directional lights
    float3 Result = float3(0, 0, 0);
    
     // perform perspective divide
    float3 projCoords = In.PositionInLightSpace.xyz / In.PositionInLightSpace.w;
    
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    for (int i = 0; i < u_DirectionalLightCount; i++)
    {
        //Result += CalculateDirectionalLight2(u_DirectionalLights[i], Normal, ViewDir, Shininess, In.Color.rgb, ShadowValue);
    }
    
    // Phase 2: Point lights
    for (int j = 0; j < u_PointLightCount; j++)
    {
        //Result += CalculatePointLight(u_PointLights[j], Normal, ViewDir, Shininess, In.WorldPosition.xyz, In.Color.rgb);
    }
    
    Result = float4(In.Color.rgb * (1.0 - ShadowValue), 1.0f);
    
    //return float4(projCoords, 1.0);
    return float4(Result, 1.0);
}