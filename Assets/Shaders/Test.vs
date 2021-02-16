struct VSIn
{
    float4 position : POSITION;
    float4 color : COLOR0;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

cbuffer FrameData : register(b0)
{
    matrix worldMat;
    matrix viewMat;
    matrix projMat;
}

VSOut main(VSIn input)
{
    VSOut output;
    output.position = mul(worldMat, input.position);
    output.position = mul(viewMat, output.position);
    output.position = mul(projMat, output.position);
    output.color = input.color;

	return output;
}