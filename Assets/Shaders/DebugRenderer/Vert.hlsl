struct VSIn
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer FrameData : register(b0)
{
    float4x4 pv;
}

VSOut main(VSIn input)
{
    VSOut output;
    output.position = mul(pv, float4(input.position, 1.f));
    output.color = float4(input.color, 1.f);

	return output;
}