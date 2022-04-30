#include "Basic.hlsli"

// 顶点着色器
VertexOutNormalMap VS(InstancePosNormalTangentTex vIn)
{
    VertexOutNormalMap vOut;
    
    vector posW = BallTransform(mul(float4(vIn.PosL, 1.0f), vIn.World),g_EyePosW);
    matrix viewProj = mul(g_View, g_Proj);

    vOut.PosW = posW.xyz;
    vOut.PosH = mul(posW, viewProj);
    vOut.NormalW = mul(vIn.NormalL, (float3x3) vIn.WorldInvTranspose);
    vOut.TangentW = mul(vIn.TangentL, vIn.World);
    vOut.Tex = vIn.Tex;
    vOut.ShadowPosH = mul(posW, g_ShadowTransform);
    return vOut;
}
