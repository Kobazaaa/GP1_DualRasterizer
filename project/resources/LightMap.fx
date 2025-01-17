float4x4 gWorldViewProj : WorldViewProjection;


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
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
};


//--------------------------------------------------
//   Vertex Shader
//--------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
       
    return output;
}
float PS(VS_OUTPUT input) : SV_Depth
{
    // The depth value is automatically written to the depth buffer
    return input.Position.z; // Z-depth value from the light's view space
}


//--------------------------------------------------
//   Technique
//--------------------------------------------------
technique11 ShadowMap
{
    pass P0
    {
        SetRasterizerState      (gRasterizerState);
        SetDepthStencilState    (gDepthStencilState, 0);
        SetBlendState           (gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

        SetVertexShader         (CompileShader(vs_5_0, VS()));
        SetGeometryShader       (NULL);
        SetPixelShader          (CompileShader(ps_5_0, PS()));
    }

}
