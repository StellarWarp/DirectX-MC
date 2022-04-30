#include "Basic.hlsli"

// 顶点着色器
VertexOutBasic VS(VertexPosNormalTex vIn)
{
    VertexOutBasic vOut;
    

    vector posW = BallTransform(mul(float4(vIn.PosL, 1.0f), g_World), g_EyePosW);
    matrix viewProj = mul(g_View, g_Proj);
    //vector posW = mul(float4(vIn.PosL, 1.0f), g_World);
    
    //vIn.PosL = BallTransform(float4(vIn.PosL, 1.0f), g_EyePosW);
    
    vOut.PosW = posW.xyz;
    //vOut.PosH = mul(posW, g_View);
    vOut.PosH = mul(posW, viewProj);
    
    vOut.NormalW = mul(vIn.NormalL, (float3x3) g_WorldInvTranspose);
    vOut.Tex = vIn.Tex;
    vOut.ShadowPosH = mul(posW, g_ShadowTransform);
    return vOut;
}
