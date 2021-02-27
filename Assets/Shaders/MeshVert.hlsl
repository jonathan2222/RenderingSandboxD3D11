struct VSIn
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : TEXCOORD;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer MeshData : register(b0)
{
    float4x4 worldMat;
}

cbuffer FrameData : register(b1)
{
    float4x4 viewMat;
    float4x4 projMat;
}

VSOut main(VSIn input)
{
    VSOut output;
    output.position = mul(worldMat, float4(input.position, 1.f));
    output.position = mul(viewMat, output.position);
    output.position = mul(projMat, output.position);

    output.normal = mul(worldMat, float4(input.normal, 0.f));

    output.uv = input.uv;

	return output;
}