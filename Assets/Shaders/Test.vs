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

VSOut main(VSIn input)
{
    VSOut output;
    output.position = input.position;
    output.color = input.color;

	return output;
}