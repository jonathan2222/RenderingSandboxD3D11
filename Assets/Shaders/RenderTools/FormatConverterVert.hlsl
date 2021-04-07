struct VSIn
{
    float2 position : POSITION; // This is not used.
    uint vertexID : SV_VertexID;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

VSOut main(VSIn input)
{
    float x = -1.f + (float)((input.vertexID & 1) << 2);
    float y = -1.f + (float)((input.vertexID & 2) << 1);
    VSOut output;
    output.position = float4(x, y, 0.5f, 1.f);
    output.uv = float2((x+1.f)*.5f, 1.f - (y+1.f)*.5f);
	return output;
}