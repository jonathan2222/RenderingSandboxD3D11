struct PSIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 uv : TEXCOORD;
};

cbuffer MaterialData : register(b0)
{
    float4 materialInfo; // x: UseCombined, y: debug draw index, z: not used, w: not used
    /*
    Debug draw index:
        0: Normal rendering
        1: Only albedo
        2: Only normal
        3: Only ao
        4: Only metallic
        5: Only roughness
        6: Only combined metallic-roughness (If avaliable)
    */
}

Texture2D       albedoTexture : register(t0);
Texture2D       normalTexture : register(t1);
Texture2D       aoTexture : register(t2);
Texture2D       metallicTexture : register(t3);
Texture2D       roughnessTexture : register(t4);
Texture2D       metallicRoughnessTexture : register(t5);
SamplerState    texSampler;

struct MaterialData
{
    float3 albedo;
    float3 normal;
    float3 ao;
    float3 metallic;
    float3 roughness;
    float3 metallicRoughness;
};

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

#define FLOAT_EQUAL(a, b) abs(a-b)<0.00001f

float3 DebugTextures(MaterialData materialData)
{
    if(FLOAT_EQUAL(materialInfo.y, 1.f))
    {
        return materialData.albedo;
    }
    else if(FLOAT_EQUAL(materialInfo.y, 2.f))
    {
        return materialData.normal*0.5f + 0.5f;
    }
    else if(FLOAT_EQUAL(materialInfo.y, 3.f))
    {
        return materialData.ao;
    }
    else if(FLOAT_EQUAL(materialInfo.y, 4.f))
    {
        return materialData.metallic;
    }
    else if(FLOAT_EQUAL(materialInfo.y, 5.f))
    {
        return materialData.roughness;
    }
    else if(FLOAT_EQUAL(materialInfo.y, 6.f))
    {
        return materialData.metallicRoughness;
    }
    else
    {
        return float3(1.f, 0.f, 1.f);
    }
}

float4 main(PSIn input) : SV_TARGET
{
    MaterialData materialData;
    materialData.albedo                 = albedoTexture.Sample(texSampler, input.uv).rgb;
    materialData.ao                     = aoTexture.Sample(texSampler, input.uv).rgb;
    materialData.metallic               = metallicTexture.Sample(texSampler, input.uv).rgb;
    materialData.roughness              = roughnessTexture.Sample(texSampler, input.uv).rgb;
    materialData.metallicRoughness      = metallicRoughnessTexture.Sample(texSampler, input.uv).rgb;

    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    input.bitangent = normalize(input.bitangent);

    materialData.normal = CalcNormal(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz, input.uv);

    if(materialInfo.y > 0.5f)
        return float4(DebugTextures(materialData), 1.f);

    float3 lightDir = normalize(float3(-1.f, -1.f, -0.5f));
    float3 ambient = float3(0.1, 0.1, 0.1);
    float diffuseFactor = max(dot(-lightDir, materialData.normal), 0.f);
    float3 diffuse = float3(diffuseFactor, diffuseFactor, diffuseFactor);
    float3 color = materialData.albedo * (ambient + diffuse);
	return float4(color, 1.0f);
}