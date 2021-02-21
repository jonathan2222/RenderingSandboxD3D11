struct DomainIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct HullPatchOut
{
    float edgeTesselation[3] : SV_TessFactor;
    float insideTesselation[1] : SV_InsideTessFactor;
};

struct DomainOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer FrameData : register(b0)
{
    float4x4 viewMat;
    float4x4 projMat;
    float4 info;
}

float3x3 MakeMatrix(float4 v1, float4 v2, float4 v3)
{
    return float3x3(v1.xyz, v2.xyz, v3.xyz);
}

// Project q onto the plane (p, n)
float4 Proj(float4 q, float4 p, float4 n)
{
    return q - dot(q-p, n) * n;
}

Texture2D       normalTexture : register(t0);
Texture2D       displacementTexture : register(t1);
SamplerState    texSampler;

[domain("tri")]
DomainOut main(HullPatchOut input, float3 uvw : SV_DomainLocation, const OutputPatch<DomainIn, 3> patch)
{
    float3x3 matP = MakeMatrix(patch[0].position, patch[1].position, patch[2].position);
    float3x3 matN = MakeMatrix(patch[0].normal, patch[1].normal, patch[2].normal);

    float4 q = float4(mul(uvw, matP), 1.f);

    float4 p0 = Proj(q, patch[0].position, patch[0].normal);
    float4 p1 = Proj(q, patch[1].position, patch[1].normal);
    float4 p2 = Proj(q, patch[2].position, patch[2].normal);
    float3x3 matPP = MakeMatrix(p0, p1, p2);

    DomainOut output;
    output.normal = float4(mul(uvw, matN), 0.f);
    output.uv = patch[0].uv * uvw.x + patch[1].uv * uvw.y + patch[2].uv * uvw.z;

    float displacement = displacementTexture.SampleLevel(texSampler, output.uv, 0).x * info.y;

    output.position = lerp(q, float4(mul(uvw, matPP), 1.f), info.x);
    output.position = output.position + float4(output.normal.xyz*displacement, 0.f);
    output.position = mul(viewMat, output.position);
    output.position = mul(projMat, output.position);
    return output;
}