struct PSIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float4x4 TBN : TBN_MATRIX;
    float2 uv : TEXCOORD;
};

Texture2D       albedoTexture : register(t0);
Texture2D       normalTexture : register(t1);
SamplerState    texSampler;

float4 main(PSIn input) : SV_TARGET
{
    float3 textureColor = albedoTexture.Sample(texSampler, input.uv).rgb;

    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    input.bitangent = normalize(input.bitangent);

    float3 normal = normalTexture.SampleLevel(texSampler, input.uv, 0).xyz;
    //normal = normalize(normal*2.f - 1.f);
    //normal = mul((float3x3)input.TBN, normal);

	return float4(normal, 1.0f);
}