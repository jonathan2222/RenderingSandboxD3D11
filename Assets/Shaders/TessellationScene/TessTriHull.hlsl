struct HullIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct HullOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct HullPatchOut
{
    float edgeTesselation[3] : SV_TessFactor;
    float insideTesselation[1] : SV_InsideTessFactor;
};

cbuffer FrameData : register(b0)
{
    float4 tessFactors;
}

// Specifies the 'domain' of the primitive being tessellated.
[domain("tri")]

// Specifies the method to tessellate with.
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]

// Specifies the name of the patch constant function.
[patchconstantfunc("HSPerPatch")]

// Specifies the largest tessellation factor that the patch constant
// function will be able to produce. This is optional and provided as a
// hint to the driver.
//[maxtessfactor(5)]

HullOut main(InputPatch<HullIn, 3> ip, uint i : SV_OutputControlPointID)
{
    HullOut output;
    output.position     = ip[i].position;
    output.normal       = ip[i].normal;
    output.tangent      = ip[i].tangent;
    output.uv           = ip[i].uv;
    return output;
}

HullPatchOut HSPerPatch(InputPatch<HullIn, 3> ip, uint patchID : SV_PrimitiveID)
{
    HullPatchOut output;

    output.edgeTesselation[0] = tessFactors.x;
    output.edgeTesselation[1] = tessFactors.x;
    output.edgeTesselation[2] = tessFactors.x;

    output.insideTesselation[0] = tessFactors.y;

    return output;
}

