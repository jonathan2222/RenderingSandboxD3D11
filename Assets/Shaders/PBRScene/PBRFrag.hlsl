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

#define FLAG_IBL 8

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
TextureCube     irradianceMap : register(t6);
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

struct PBRMaterial
{
    float3 normal;
    float3 invLightDir;
    float3 invViewDir;
    float roughness;
    float metallic;
    float kd;
    float3 albedo;
    float k; // Remapping of roughness (Dependent of the light (IBL or direct light))
};

/*
    The Normal Distribution Function (NDF) for the use in the BRDF.
    This uses the Trowbridge-Reitz GGX function.
    @param n: Normal
    @param h: Halfway vector between the surface normal and light direction.
    @param roughness: Roughness factor [0, 1]
*/
float DistributionGGX(float3 n, float3 h, float roughness)
{
    float a         = roughness*roughness;
    float a2        = a*a;
    float nDotH     = max(dot(n, h), 0.f);
    float nDotH2    = nDotH*nDotH;

    float denom     = nDotH2 * (a2 - 1.f) + 1.f;
    denom           = PI * denom * denom;

    return a2 / denom;
}

/*
    Schlick-GGX function for calculating part of the Geometry function.
    @param nDotV: The dot-product between the normal and the view direction.
    @param k: Remapping of the roughness parameter a. This is calculated differently if the light is from an IBL or direct lighting.
        Direct lighting:    k = (a + 1)^2 / 8
        IBL:                k = a^2 / 2
*/
float GeometrySchlickGGX(float nDotV, float roughness)
{
    float k = roughness;
    k = roughness + 1.f;
    k = (k*k) / 8.f;

    float denom = nDotV * (1.f - k) + k;
    return nDotV / denom;
}

/*
    Geometry function (G) for the use in the BRDF.
    This uses the Smith's Schlick-GGX function.
    @param nDotV: The dot-product between the normal and the view direction.
    @param k: Remapping of the roughness parameter a. This is calculated differently if the light is from an IBL or direct lighting.
        Direct lighting:    k = (a + 1)^2 / 8
        IBL:                k = a^2 / 2
*/
float GeometrySmith(float nDotV, float nDotL, float roughness)
{
    float ggx1 = GeometrySchlickGGX(nDotV, roughness);
    float ggx2 = GeometrySchlickGGX(nDotL, roughness);
    return ggx1 * ggx2;
}

/*
    Fresnel function (F) for use in the BRDF.
    This uses the Fresnel-Schlick approximation.
    @param cosTheta: The angle between the normal and teh halfway (or view) direction.
    @param F0: Base reflectivity of the surface.
    @param roughness: The roughness of the material.
*/
float3 FresnelSchlick(float cosTheta, float3 F0, float roughness)
{
    float c = min(cosTheta, 1.f);
    float3 v = float3(1.f - roughness, 1.f - roughness, 1.f - roughness);
    return F0 + (max(v, F0) - F0) * pow(max(1.f - c, 0.f), 5.f);
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

    PBRMaterial material;
    material.normal = materialData.normal;
    material.invViewDir = normalize(cameraPos.xyz - input.worldPosition.xyz);
    material.roughness = max(GetAlbedoColor(materialData), 0.01f);
    material.metallic = GetMetallicData(materialData);
    material.albedo = GetAlbedoColor(materialData);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, material.albedo, material.metallic);

    // Loop through all light sources
    float3 Lo = float3(0.f, 0.f, 0.f);
    {
        float3 lightColor = float3(1.f, 1.f, 1.f);
        float3 lightDir = normalize(input.worldPosition.xyz - lightPos.xyz);
        material.invLightDir = -lightDir;

        float nDotVMax = max(dot(material.normal, material.invViewDir), 0.f);
        float nDotL = dot(material.normal, material.invLightDir);
        float nDotLMax = max(nDotL, 0.f);

        float3 h = normalize(material.invLightDir + material.normal);

        float distance = length(lightPos.xyz - input.worldPosition.xyz);
        float attenuation = 1.f / (distance*distance);
        float3 radiance = lightColor * attenuation;

        float cosTheta = dot(material.normal, h);
        float NDF   = DistributionGGX(material.normal, h, material.roughness);
        float G     = GeometrySmith(nDotVMax, nDotLMax, material.roughness);
        float3 F    = FresnelSchlick(cosTheta, F0, material.roughness);

        float3 ks = F;
        material.kd = float3(1.f, 1.f, 1.f) - ks;
        material.kd *= 1.f - material.metallic;

        // BRDF
        float denom = 4.f*nDotLMax*nDotVMax;
        float3 specular = NDF*F*G / max(denom, 0.0001f);

        Lo += (material.kd * material.albedo / PI + specular) * radiance * nDotL;
    }

    float3 ambient = float3(0.f, 0.f, 0.f);
    if(GET_FLAGS & FLAG_IBL)
    {
        float3 kS = FresnelSchlick(max(dot(material.normal, material.invViewDir), 0.f), F0, material.roughness);
        float3 kD = 1.f - kS;
        float3 irradiance = irradianceMap.Sample(linearSampler, material.normal).rgb;
        float3 diffuse = irradiance * material.albedo;
        ambient = kD * diffuse * GetAOData(materialData);
    }
    else
    {
        ambient = float3(0.03f, 0.03f, 0.03f) * material.albedo * GetAOData(materialData);
    }

    float3 color = ambient + Lo;

    color = color / (color + 1.f);
    color = pow(color, 1.f/2.2f);
    
	return float4(color, 1.0f);
}