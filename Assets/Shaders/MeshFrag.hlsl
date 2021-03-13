struct PSIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 uv : TEXCOORD;
};

Texture2D       albedoTexture : register(t0);
Texture2D       normalTexture : register(t1);
SamplerState    texSampler;

float3 CalcNormal(float3 t, float3 b, float3 n, float2 uv)
{
    float3 normal = n;
    if(abs(dot(t, t)) > 0.001f)
    {
        float3x3 TBN = float3x3(t, b, n);
        TBN = transpose(TBN);

        normal = normalTexture.SampleLevel(texSampler, uv, 0).xyz;
        normal = normalize(normal*2.f - 1.f);
        normal = mul(TBN, normal);
    }

    return normal;
}

float4 main(PSIn input) : SV_TARGET
{
    float3 textureColor = albedoTexture.Sample(texSampler, input.uv).rgb;

    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    input.bitangent = normalize(input.bitangent);

    float3 normal = CalcNormal(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz, input.uv);

    float3 lightDir = normalize(float3(-1.f, -1.f, -0.5f));
    float3 ambient = float3(0.1, 0.1, 0.1);
    float diffuseFactor = max(dot(-lightDir, normal), 0.f);
    float3 diffuse = float3(diffuseFactor, diffuseFactor, diffuseFactor);
    float3 color = textureColor * (ambient + diffuse);
	return float4(color, 1.0f);
}