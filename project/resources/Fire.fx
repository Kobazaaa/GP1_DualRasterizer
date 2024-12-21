//--------------------------------------------------
//   Globals
//--------------------------------------------------
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gWorldViewProj : WorldViewProjection;

Texture2D gDiffuseMap : DiffuseMap;

//--------------------------------------------------
//   Sampler States
//--------------------------------------------------
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
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0f;
};

//--------------------------------------------------
//   Depth Stencil State
//--------------------------------------------------
DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;
};

//--------------------------------------------------
//   Input/Output Structs
//--------------------------------------------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 UV : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : WORLD;
    float3 Color : COLOR;
    float2 UV : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

//--------------------------------------------------
//   Vertex Shader
//--------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
    output.Color = input.Color;
    output.UV = input.UV;
    output.normal  = mul(normalize(input.normal), (float3x3) gWorldMatrix);
    output.tangent = mul(normalize(input.tangent), (float3x3) gWorldMatrix);
    return output;
}

//--------------------------------------------------
//   Pixel Shader
//--------------------------------------------------
float4 PSP(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(samPoint, input.UV);
}
float4 PSL(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(samLinear, input.UV);
}
float4 PSA(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(samAnisotropic, input.UV);
}

//--------------------------------------------------
//   Technique
//--------------------------------------------------
technique11 PointSamplingTechnique
{
    pass P0
    {
        SetRasterizerState      (gRasterizerState);
        SetDepthStencilState    (gDepthStencilState, 0);
        SetBlendState           (gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader         ( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader       ( NULL );
        SetPixelShader          ( CompileShader( ps_5_0, PSP() ) );
    }
}
technique11 LinearSamplingTechnique
{
    pass P0
    {
        SetRasterizerState      (gRasterizerState);
        SetDepthStencilState    (gDepthStencilState, 0);
        SetBlendState           (gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader         (CompileShader(vs_5_0, VS()));
        SetGeometryShader       (NULL);
        SetPixelShader          (CompileShader(ps_5_0, PSL()));
    }

}
technique11 AnisotropicSamplingTechnique
{
    pass P0
    {
        SetRasterizerState      (gRasterizerState);
        SetDepthStencilState    (gDepthStencilState, 0);
        SetBlendState           (gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader         (CompileShader(vs_5_0, VS()));
        SetGeometryShader       (NULL);
        SetPixelShader          (CompileShader(ps_5_0, PSA()));
    }
}
