//--------------------------------------------------
//   Globals
//--------------------------------------------------
float4x4 gWorldMatrix       : WorldMatrix;
float4x4 gWorldViewProj     : WorldViewProjection;
float3 gCameraPos           : CAMERA;

Texture2D gDiffuseMap       : DiffuseMap;
Texture2D gNormalMap        : NormalMap;
Texture2D gSpecularMap      : SpecularMap;
Texture2D gGlossinessMap    : GlossinessMap;

float gPI = 3.14159265358979323846f;
float gLIGHT_INTENSITY = 7.f;
float gSHININESS = 25.f;
float3 gLIGHT_DIR = { 0.577f, -0.577f, 0.577f };
float4 gAMBIENT = { 0.03f, 0.03f, 0.03f, 0.f };

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
    CullMode = back;
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
    float3 Position         : POSITION;
    float3 Color            : COLOR;
    float2 UV               : TEXCOORD;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
};
struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float4 WorldPosition    : WORLD;
    float3 Color            : COLOR;
    float2 UV               : TEXCOORD;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
};

//--------------------------------------------------
//   Helpful Functions
//--------------------------------------------------
float CalculateObservedArea(const float3 normal, const float3 lightDirection)
{
    return dot(normal, lightDirection);
}
float4 GetDiffuseColor(const float2 UV, const SamplerState samplerState)
{
    return gDiffuseMap.Sample(samplerState, UV) * gLIGHT_INTENSITY / gPI;
}
float4 GetSpecularColor(VS_OUTPUT input, const SamplerState samplerState)
{
    float3 invViewDir = normalize(gCameraPos - input.WorldPosition.xyz);
    
    float ks = gSpecularMap.Sample(samplerState, input.UV).r;
    float exp = gGlossinessMap.Sample(samplerState, input.UV).r * gSHININESS;
    
    float3 ref = reflect(gLIGHT_DIR, input.normal);
    float cosAlpha = max(dot(ref, invViewDir), 0);
    
    return float4(1, 1, 1, 1) * ks * pow(cosAlpha, exp);
}


float4 ShadePixel(VS_OUTPUT input, SamplerState samplerState)
{
    // OA
    float observedArea = CalculateObservedArea(input.normal, -gLIGHT_DIR);
    
    // Lambert Diffuse Color
    float4 diffuse = GetDiffuseColor(input.UV, samplerState);
    
    // Specular Phong
    float4 specular = GetSpecularColor(input, samplerState);
    
    return (diffuse + specular + gAMBIENT) * observedArea;
}

//--------------------------------------------------
//   Vertex Shader
//--------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output        = (VS_OUTPUT)0;
    
    // Positions
    output.Position         = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.WorldPosition    = mul(float4(input.Position, 1.f), gWorldMatrix);
   
    // Color
    output.Color            = input.Color;
    
    // UV
    output.UV               = input.UV;
    
    // Normal
    output.normal           = mul(normalize(input.normal), (float3x3) gWorldMatrix);
    output.tangent          = mul(normalize(input.tangent), (float3x3) gWorldMatrix);
    
    return output;
}

//--------------------------------------------------
//   Pixel Shader
//--------------------------------------------------
float4 PSP(VS_OUTPUT input) : SV_TARGET
{
    return ShadePixel(input, samPoint);
}
float4 PSL(VS_OUTPUT input) : SV_TARGET
{
    return ShadePixel(input, samLinear);
}
float4 PSA(VS_OUTPUT input) : SV_TARGET
{
    return ShadePixel(input, samAnisotropic);
}

//--------------------------------------------------
//   Technique
//--------------------------------------------------
technique11 PointSamplingTechnique
{
    pass P0
    {
        // Setting the rasterizer state gets done in the C++ code, so that we can dynamically change the cull mode
        //SetRasterizerState      (gRasterizerState);
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
        // Setting the rasterizer state gets done in the C++ code, so that we can dynamically change the cull mode
        //SetRasterizerState      (gRasterizerState);
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
        // Setting the rasterizer state gets done in the C++ code, so that we can dynamically change the cull mode
        //SetRasterizerState      (gRasterizerState);
        SetDepthStencilState    (gDepthStencilState, 0);
        SetBlendState           (gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader         (CompileShader(vs_5_0, VS()));
        SetGeometryShader       (NULL);
        SetPixelShader          (CompileShader(ps_5_0, PSA()));
    }
}
