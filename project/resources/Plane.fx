//--------------------------------------------------
//   Globals
//--------------------------------------------------
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gLightViewProj : LightViewProj;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gShadowMap : ShadowMap;

float gSHADOW_BIAS = 0.005f;
float gSHADOW_MULTIPLIER = 0.5f;

//--------------------------------------------------
//   Sampler States
//--------------------------------------------------
SamplerState samShadow
{
    Filter = ANISOTROPIC;
    AddressU = Border;
    AddressV = Border;

    BorderColor = float4(100.0f, 100.0f, 100.0f, 100.0f);
};
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

//--------------------------------------------------
//   Rasterizer State
//--------------------------------------------------
RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false;
};

//--------------------------------------------------
//   Blend State
//--------------------------------------------------
BlendState gBlendState
{};

//--------------------------------------------------
//   Depth Stencil State
//--------------------------------------------------
DepthStencilState gDepthStencilState
{};

//--------------------------------------------------
//   Input/Output Structs
//--------------------------------------------------
struct VS_INPUT
{
    float3 Position     : POSITION;
    float3 Color        : COLOR;
    float2 UV           : TEXCOORD;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
};
struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float4 WorldPosition    : WORLD;
    float3 Color            : COLOR;
    float2 UV               : TEXCOORD;
    float4 ShadowPos        : TEXCOORD1;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
};

//--------------------------------------------------
//   Vertex Shader
//--------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    // Positions
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
   
    // Color
    output.Color = input.Color;
    
    // UV
    output.UV = input.UV;
    float4 shadowPos = mul(float4(output.WorldPosition.xyz, 1), gLightViewProj);
    shadowPos.xy /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * 0.5f + 0.5f;
    output.ShadowPos = shadowPos; // Normalized device coordinates
    output.ShadowPos.y = 1 - output.ShadowPos.y;

    // Normal
    output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
    
    return output;
}

//--------------------------------------------------
//   Pixel Shader
//--------------------------------------------------
float4 GetColor(VS_OUTPUT input, SamplerState samState)
{
    float shadowDepth = gShadowMap.Sample(samShadow, input.ShadowPos.xy).r;
    float currentDepth = input.ShadowPos.z / input.ShadowPos.w;
    
    float mult;
    if (currentDepth - gSHADOW_BIAS > shadowDepth)
        mult = gSHADOW_MULTIPLIER;
    else
        mult = 1.f;
    
    return gDiffuseMap.Sample(samPoint, input.UV) * mult;
}

float4 PSP(VS_OUTPUT input) : SV_TARGET
{
    return GetColor(input, samPoint);
}
float4 PSL(VS_OUTPUT input) : SV_TARGET
{
    return GetColor(input, samLinear);
}
float4 PSA(VS_OUTPUT input) : SV_TARGET
{
    return GetColor(input, samAnisotropic);
}

//--------------------------------------------------
//   Technique
//--------------------------------------------------
technique11 PointSamplingTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSP()));
    }
}
technique11 LinearSamplingTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSL()));
    }

}
technique11 AnisotropicSamplingTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSA()));
    }
}
