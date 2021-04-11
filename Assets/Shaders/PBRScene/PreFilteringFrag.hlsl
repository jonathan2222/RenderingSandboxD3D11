struct PSIn
{
    float4 position : SV_POSITION;
    float3 dir : DIRECTION;
};

TextureCube     environmentMap : register(t0);
SamplerState    linearSampler;

cbuffer InfoData : register(b0)
{
    float4 info;
}

static const float PI = 3.14159265359f;

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return ((float)bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
    return float2((float)i/(float)N, RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0f * PI * Xi.x;
    float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a*a - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    float3 up        = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 tangent   = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float DistributionGGX(float3 nDotH, float roughness)
{
    float a         = roughness*roughness;
    float a2        = a*a;
    float nDotH2    = nDotH*nDotH;

    float denom     = nDotH2 * (a2 - 1.f) + 1.f;
    denom           = PI * denom * denom;

    return a2 / denom;
}

SamplerState g_SamplePoint
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 main(PSIn input) : SV_TARGET
{
    float3 normal = normalize(input.dir);
    float3 R = normal;
    float3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0f;   
    float3 prefilteredColor = float3(0.f, 0.f, 0.f);     
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H  = ImportanceSampleGGX(Xi, normal, info.x);
        float vDotH = dot(V, H);
        float3 L  = normalize(2.0 * vDotH * H - V);

        float NdotL = max(dot(normal, L), 0.0);
        if(NdotL > 0.0)
        {
            float nDotH = max(dot(normal, H), 0.f);
            float D = DistributionGGX(nDotH, info.x);
            float pdf = (D * nDotH / (4.f - vDotH)) + 0.0001f;

            float resolution = info.y; // resolution of source cubemap (per face)
            float saTexel = 4.f * PI / (6.f * resolution * resolution);
            float saSample = 1.f / ((float)SAMPLE_COUNT * pdf + 0.0001f);
            float mipLevel = info.x == 0.f ? 0.f : 0.5f * log2(saSample / saTexel);

            prefilteredColor += environmentMap.SampleLevel(g_SamplePoint, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

	return float4(prefilteredColor, 1.0f);
}