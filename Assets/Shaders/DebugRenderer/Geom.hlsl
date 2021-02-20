struct GSIn
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct GSOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer FrameData : register(b0)
{
    float4x4 proj;
    float pointSize;
}

[instance(1)]
[maxvertexcount(4)]
void main(point GSIn input[1], inout TriangleStream<GSOut> OutputStream)
{
    float4 pos = input[0].position;

    GSOut output;
    output.color = input[0].color; 

    // BL
    float2 p = pos.xy - float2(-0.5f, -0.5f) * pointSize;
    output.position = mul(proj, float4(p, pos.zw));
    OutputStream.Append(output);

    // TL
    p = pos.xy - float2(-0.5f, 0.5f) * pointSize;
    output.position = mul(proj, float4(p, pos.zw));
    OutputStream.Append(output);

    // BR
    p = pos.xy - float2(0.5f, -0.5f) * pointSize;
    output.position = mul(proj, float4(p, pos.zw));
    OutputStream.Append(output);

    // TR
    p = pos.xy - float2(0.5f, 0.5f) * pointSize;
    output.position = mul(proj, float4(p, pos.zw));
    OutputStream.Append(output);

    OutputStream.RestartStrip();
}