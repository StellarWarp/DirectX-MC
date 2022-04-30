#include "Sky.hlsli"

float4 PS(VertexPosHL pIn) : SV_Target
{
    if (g_renderSun)
        return (g_SunColor,1.0);
    else
        return g_TexCube.Sample(g_Sam, pIn.PosL);
}
