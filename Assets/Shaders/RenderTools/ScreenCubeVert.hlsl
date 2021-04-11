struct VSIn
{
    float2 position : POSITION; // This is not used.
    uint vertexID : SV_VertexID;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float3 dir : DIRECTION;
};

cbuffer FrameData : register(b0)
{
    float4x4 viewMat;
    float4x4 projMat;
}

VSOut main(VSIn input)
{
    // From: members.chello.at/~easyfilter/bresenham.html
    float x = -1.f + (float)((input.vertexID & 1) << 2);
    float y = -1.f + (float)((input.vertexID & 2) << 1);
    VSOut output;
    float4 position = float4(x, y, -1.f, 1.f);
    output.position = mul(projMat, position);
    output.dir = mul(viewMat, position).xyz;
	return output;
}