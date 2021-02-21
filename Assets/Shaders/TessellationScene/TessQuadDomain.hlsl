struct DomainIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct HullPatchOut
{
    float edgeTesselation[4] : SV_TessFactor;
    float insideTesselation[2] : SV_InsideTessFactor;
};

struct DomainOut
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float4 normal : NORMAL;
    float3x3 TBN : TBN_MATRIX;
    float2 uv : TEXCOORD;
};

cbuffer FrameData : register(b0)
{
    float4x4 worldMat;
    float4x4 viewMat;
    float4x4 projMat;
    float4 info;
}

// Project q onto the plane (p, n)
float4 Proj(float4 q, float4 p, float4 n)
{
    return q - dot(q-p, n) * n;
}

Texture2D       displacementTexture : register(t0);
SamplerState    texSampler;

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
    output.uv = lerp(lerp(patch[0].uv, patch[1].uv, uv.x),
                    lerp(patch[2].uv, patch[3].uv, uv.x), uv.y);

    float4 tangent = normalize(lerp(lerp(patch[0].tangent, patch[1].tangent, uv.x),
                                    lerp(patch[2].tangent, patch[3].tangent, uv.x), uv.y));

    output.normal = lerp(lerp(patch[0].normal, patch[1].normal, uv.x),
                        lerp(patch[2].normal, patch[3].normal, uv.x), uv.y);
    float3 bitangent = normalize(cross(output.normal.xyz, tangent.xyz));
    output.TBN = float3x3(tangent.xyz, bitangent, output.normal.xyz);
    output.TBN = transpose(output.TBN);

    float displacement = displacementTexture.SampleLevel(texSampler, output.uv, 0).x * info.y;
    
    output.position = lerp(q, pp, info.x);
    output.position = output.position + float4(output.normal.xyz*displacement, 0.f);
    output.worldPosition = output.position;
    output.position = mul(viewMat, output.position);
    output.position = mul(projMat, output.position);

    return output;
}