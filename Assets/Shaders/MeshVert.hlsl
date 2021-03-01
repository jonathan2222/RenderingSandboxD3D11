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
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float4x4 TBN : TBN_MATRIX;
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
    output.tangent = mul(worldMat, float4(input.tangent, 0.f));
    output.bitangent = mul(worldMat, float4(input.bitangent, 0.f));

    output.TBN = float4x4(output.tangent, output.bitangent, output.normal, float4(0.f, 0.f, 0.f, 1.f));
    output.TBN = transpose(output.TBN);

    output.uv = input.uv;

	return output;
}