#include "Light.hlsl"

cbuffer root_constants : register(b0)
{
    row_major float4x4 LightSpaceMatrix;
};

struct vertex_shader_input
{
    float4 VertexPosition : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
};

struct pixel_shader_input
{
    float4 Position : SV_POSITION;
};

pixel_shader_input VSMain(vertex_shader_input In)
{
    pixel_shader_input Out;
    
    Out.Position = mul(LightSpaceMatrix, In.VertexPosition);

    return Out;
}

void PSMain(pixel_shader_input In)
{

}