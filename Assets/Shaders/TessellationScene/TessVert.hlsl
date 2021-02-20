struct VSIn
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
};

cbuffer FrameData : register(b0)
{
    float4x4 worldMat;
}

VSOut main(VSIn input)
{
    VSOut output;
    output.position = mul(worldMat, float4(input.position, 1.f));

    output.normal = mul(worldMat, float4(input.normal, 0.f));

	return output;
}