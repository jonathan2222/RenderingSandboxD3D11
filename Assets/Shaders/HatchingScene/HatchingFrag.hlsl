struct PSIn
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float2 uv : TEXCOORD;
};

cbuffer MaterialData : register(b0)
{
    float4 materialInfo; // x: UseCombined, y: debug draw index, z: preFilterMaxLOD, w: not used
    /*
    Debug draw index:
        0: Normal rendering
        1: Only albedo
        2: Only normal
        3: Only ao
        4: Only metallic
        5: Only roughness
        6: Only combined metallic-roughness (If avaliable)
        7: -------- Not used --------

    Additional flags on the draw index (Three flags: 8, 16 and 32):
        8: Use Diffuse IBL (Add contribution from an irradiance map)
        16: -------- Not used --------
        32: -------- Not used --------
    */
}

#define INDEX_ALBEDO 1
#define INDEX_NORMAL 2
#define INDEX_AO 3
#define INDEX_METALLIC 4
#define INDEX_ROUGHNESS 5
#define INDEX_METALLIC_ROUGHNESS 6

#define FLAG_DIFF_IBL 8
#define FLAG_SPEC_IBL 16

#define INDEX(a) ((uint)a & 7)
#define IS_INDEX(i) (INDEX(materialInfo.y) == i)
#define FLAGS(a) ((uint)a & (~7))
#define GET_FLAGS FLAGS(materialInfo.y)

cbuffer CameraData : register(b1)
{
    float4 cameraPos;
    float4 lightPos;
}

Texture2D       albedoTexture : register(t0);
Texture2D       normalTexture : register(t1);
Texture2D       aoTexture : register(t2);
Texture2D       metallicTexture : register(t3);
Texture2D       roughnessTexture : register(t4);
Texture2D       metallicRoughnessTexture : register(t5);

Texture2D       hatch0Texture : register(t6);
Texture2D       hatch1Texture : register(t7);
Texture2D       hatch2Texture : register(t8);
Texture2D       hatch3Texture : register(t9);
Texture2D       hatch4Texture : register(t10);
Texture2D       hatch5Texture : register(t11);
SamplerState    linearSampler;

static const float PI = 3.14159265359f;

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

        normal = normalTexture.SampleLevel(linearSampler, uv, 0).xyz;
        normal = normalize(normal*2.f - 1.f);
        normal = mul(TBN, normal);
    }

    return normal;
}

#define FLOAT_EQUAL(a, b) abs(a-b)<0.00001f

float3 GetAlbedoColor(MaterialData materialData)
{
    return materialData.albedo;
}

float GetMetallicData(MaterialData materialData)
{
    if(materialInfo.x > 0.5f)
        return materialData.metallicRoughness.b;
    else
        return materialData.metallic.r;
}

float GetRoughnessData(MaterialData materialData)
{
    if(materialInfo.x > 0.5f)
        return materialData.metallicRoughness.g;
    else
        return materialData.roughness.r;
}

float GetAOData(MaterialData materialData)
{
    if(materialInfo.x > 0.5f)
        return materialData.metallicRoughness.r;
    else
        return materialData.ao.r;
}

float3 DebugTextures(MaterialData materialData)
{
    if(materialInfo.x > 0.5f)
    {
        if(IS_INDEX(INDEX_AO)) // AO
        {
            return float3(materialData.metallicRoughness.r, materialData.metallicRoughness.r, materialData.metallicRoughness.r);
        }
        else if(IS_INDEX(INDEX_METALLIC)) // Metallic
        {
             return float3(materialData.metallicRoughness.b, materialData.metallicRoughness.b, materialData.metallicRoughness.b);
        }
        else if(IS_INDEX(INDEX_ROUGHNESS)) // Roughness
        {
             return float3(materialData.metallicRoughness.g, materialData.metallicRoughness.g, materialData.metallicRoughness.g);
        }
    }

    if(IS_INDEX(INDEX_ALBEDO))
    {
        return  pow(materialData.albedo, 1.f/2.2f);
    }
    else if(IS_INDEX(INDEX_NORMAL))
    {
        return materialData.normal*0.5f + 0.5f;
    }
    else if(IS_INDEX(INDEX_AO))
    {
        return materialData.ao;
    }
    else if(IS_INDEX(INDEX_METALLIC))
    {
        return materialData.metallic;
    }
    else if(IS_INDEX(INDEX_ROUGHNESS))
    {
        return materialData.roughness;
    }
    else if(IS_INDEX(INDEX_METALLIC_ROUGHNESS))
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
    materialData.albedo                 = pow(albedoTexture.Sample(linearSampler, input.uv).rgb, 2.2f);
    materialData.ao                     = aoTexture.Sample(linearSampler, input.uv).rgb;
    materialData.metallic               = metallicTexture.Sample(linearSampler, input.uv).rgb;
    materialData.roughness              = roughnessTexture.Sample(linearSampler, input.uv).rgb;
    materialData.metallicRoughness      = metallicRoughnessTexture.Sample(linearSampler, input.uv).rgb;

    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    input.bitangent = normalize(input.bitangent);

    materialData.normal = CalcNormal(input.tangent.xyz, input.bitangent.xyz, input.normal.xyz, input.uv);

    if(INDEX(materialInfo.y) > 0)
        return float4(DebugTextures(materialData), 1.f);

    float3 lightDir = normalize(input.worldPosition.xyz - lightPos.xyz);
    float diffuse = max(dot(materialData.normal, -lightDir), 0.f);
    float shading = diffuse + 0.1;

    float2 uv = input.uv*4.;
    float4 hatch1 = hatch0Texture.Sample(linearSampler, uv).rgba;
    float4 hatch2 = hatch1Texture.Sample(linearSampler, uv).rgba;
    float4 hatch3 = hatch2Texture.Sample(linearSampler, uv).rgba;
    float4 hatch4 = hatch3Texture.Sample(linearSampler, uv).rgba;
    float4 hatch5 = hatch4Texture.Sample(linearSampler, uv).rgba;
    float4 hatch6 = hatch5Texture.Sample(linearSampler, uv).rgba;

    float4 c = float4(1. ,1., 1., 1.);
    float step = 1. / 6.;
    if(shading <= step)
        c = lerp(hatch6, hatch5, 6. * shading);
    if(shading > step && shading <= 2. * step)
        c = lerp(hatch5, hatch4, 6. * (shading - step));
    if(shading > 2. * step && shading <= 3. * step)
        c = lerp(hatch4, hatch3, 6. * (shading - 2. * step));
    if(shading > 3. * step && shading <= 4. * step)
        c = lerp(hatch3, hatch2, 6. * (shading - 3. * step));
    if(shading > 4. * step && shading <= 5. * step)
        c = lerp(hatch2, hatch1, 6. * (shading - 4. * step));
    if(shading > 5. * step)
        c = lerp(hatch1, float4(1., 1., 1., 1.), 6. * (shading - 5. * step));

    float4 inkColor = float4(0., 0., 0., 1.f);
    float4 color = lerp(lerp(inkColor, float4(1., 1., 1., 1.), c.r), c, .5f);
	return color;
}