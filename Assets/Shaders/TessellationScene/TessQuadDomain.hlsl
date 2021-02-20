struct DomainIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
};

struct HullPatchOut
{
    float edgeTesselation[4] : SV_TessFactor;
    float insideTesselation[2] : SV_InsideTessFactor;
};

struct DomainOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
};

cbuffer FrameData : register(b0)
{
    float4x4 viewMat;
    float4x4 projMat;
    float4 info;
}

// Project q onto the plane (p, n)
float4 Proj(float4 q, float4 p, float4 n)
{
    return q - dot(q-p, n) * n;
}

[domain("quad")]
DomainOut main(HullPatchOut input, float2 uv : SV_DomainLocation, const OutputPatch<DomainIn, 4> patch)
{
    float4 q = lerp(lerp(patch[0].position, patch[1].position, uv.x),
                    lerp(patch[2].position, patch[3].position, uv.x), uv.y);

    float4 p0 = Proj(q, patch[0].position, patch[0].normal);
    float4 p1 = Proj(q, patch[1].position, patch[1].normal);
    float4 p2 = Proj(q, patch[2].position, patch[2].normal);
    float4 p3 = Proj(q, patch[3].position, patch[3].normal);
    float4 pp = lerp(lerp(p0, p1, uv.x), lerp(p2, p3, uv.x), uv.y);

    DomainOut output;
    output.position = lerp(q, pp, info.x);
    output.position = mul(viewMat, output.position);
    output.position = mul(projMat, output.position);

    output.normal = lerp(lerp(patch[0].normal, patch[1].normal, uv.x),
                        lerp(patch[2].normal, patch[3].normal, uv.x), uv.y);
    return output;
}