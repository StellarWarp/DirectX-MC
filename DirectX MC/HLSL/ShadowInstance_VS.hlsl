#include "TransformHelper.hlsli"

cbuffer CB : register(b0)
{
    matrix g_WorldViewProj;
    matrix g_ViewProj;
    float3 g_EyePosW;
    float g_Pad2;
}

struct InstancePosNormalTex
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    matrix World : World;
};

struct VertexPosHTex
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};

VertexPosHTex VS(InstancePosNormalTex vIn)
{
    VertexPosHTex vOut;
    
    float4 posW = BallTransform(mul(float4(vIn.PosL, 1.0f), vIn.World), g_EyePosW);
    
    vOut.PosH = mul(posW, g_ViewProj);
    vOut.Tex = vIn.Tex;

    return vOut;
}
