struct VSIn
{
    float4 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD;
};

cbuffer FrameData : register(b0)
{
    float4x4 worldMat;
    float4x4 viewMat;
    float4x4 projMat;
}

VSOut main(VSIn input)
{
    VSOut output;
    output.position = mul(worldMat, input.position);
    output.position = mul(viewMat, output.position);
    output.position = mul(projMat, output.position);
    output.color = input.color;
    output.uv = input.uv;

	return output;
}