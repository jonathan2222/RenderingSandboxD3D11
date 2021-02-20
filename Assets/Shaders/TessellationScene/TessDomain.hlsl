struct DomainIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
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
};

cbuffer FrameData : register(b0)
{
    float4x4 viewMat;
    float4x4 projMat;
}

[domain("tri")]
DomainOut main(HullPatchOut input, float3 uvw : SV_DomainLocation, const OutputPatch<DomainIn, 3> patch)
{
    DomainOut output;
    output.position = patch[0].position * uvw.x + patch[1].position * uvw.y + patch[2].position * uvw.z;
    output.position = mul(viewMat, output.position);
    output.position = mul(projMat, output.position);

    output.normal = patch[0].normal * uvw.x + patch[1].normal * uvw.y + patch[2].normal * uvw.z;
    return output;
}