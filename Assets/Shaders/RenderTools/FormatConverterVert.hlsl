struct VSIn
{
    float2 position : POSITION;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

VSOut main(VSIn input)
{
    VSOut output;
    output.position = float4(input.position.x, input.position.y, 0.f, 1.f);
    output.uv = float2(input.position.x+1.f, input.position.y+1.f);
	return output;
}