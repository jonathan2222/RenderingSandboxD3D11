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
    float4 localPos : POSITION;
    float2 uv : UV;
};

cbuffer MeshData : register(b0)
{
    float4x4 worldMat2;
}

cbuffer FrameData : register(b1)
{
    float4x4 worldMat; // Not used
    float4x4 viewMat;
    float4x4 projMat;
}

VSOut main(VSIn input)
{
    VSOut output;
    output.position = mul(worldMat, float4(input.position, 1.f));

    // Remove translation from the view matrix. (Make the skybox follow the player)
    float4x4 view = viewMat;
    view[0][3] = 0.f;
    view[1][3] = 0.f;
    view[2][3] = 0.f;
    output.position = mul(view, output.position);
    output.position = mul(projMat, output.position);
    output.localPos = float4(input.position, 1.f);

    output.uv = input.uv;
	return output;
}