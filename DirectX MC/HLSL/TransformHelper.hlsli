
cbuffer CB_ET : register(b6)
{
    bool ET_enable;
    float ET_r0;
}

float4 BallTransform(float4 pos,float3 eye)
{
    if (ET_enable)
    {
    
        pos.xyz -= eye;
        float4 res;
        float r = pos.y + ET_r0;
        float d = length(float2(pos.x, pos.z));
        float theta = d / r;
        float k = sin(theta) * r / d;
    
        res.x = pos.x * k;
        res.y = cos(theta) * r - ET_r0;
        res.z = pos.z * k;
        res.w = pos.w;
        res.xyz += eye;
        return res;
    }
    else
    {
        return pos;
    }
}
