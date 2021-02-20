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
}

[domain("quad")]
DomainOut main(HullPatchOut input, float2 uv : SV_DomainLocation, const OutputPatch<DomainIn, 4> patch)
{
    DomainOut output;
    output.position = lerp(lerp(patch[0].position, patch[1].position, uv.x),
                            lerp(patch[2].position, patch[3].position, uv.x), uv.y);
    output.position = mul(viewMat, output.position);
    output.position = mul(projMat, output.position);

    output.normal = lerp(lerp(patch[0].normal, patch[1].normal, uv.x),
                        lerp(patch[2].normal, patch[3].normal, uv.x), uv.y);
    return output;
}