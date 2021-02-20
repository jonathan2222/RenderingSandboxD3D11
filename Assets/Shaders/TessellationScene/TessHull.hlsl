struct HullIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
};

struct HullOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
};

struct HullPatchOut
{
    float edgeTesselation[3] : SV_TessFactor;
    float insideTesselation[1] : SV_InsideTessFactor;
};

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
    output.position = ip[i].position;
    output.normal   = ip[i].normal;
    return output;
}

HullPatchOut HSPerPatch(InputPatch<HullIn, 3> ip, uint patchID : SV_PrimitiveID)
{
    HullPatchOut output;

    output.edgeTesselation[0] = 2.0f;
    output.edgeTesselation[1] = 2.0f;
    output.edgeTesselation[2] = 2.0f;

    output.insideTesselation[0] = 2.f;

    return output;
}

