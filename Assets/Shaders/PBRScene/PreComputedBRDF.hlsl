struct PSIn
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

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

float GeometrySchlickGGX(float nDotV, float roughness)
{
    float k = roughness;
    k = (k*k) / 2.f;

    float denom = nDotV * (1.f - k) + k;
    return nDotV / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float nDotV = max(dot(N, V), 0.f);
    float nDotL = max(dot(N, L), 0.f);
    float ggx1 = GeometrySchlickGGX(nDotV, roughness);
    float ggx2 = GeometrySchlickGGX(nDotL, roughness);
    return ggx1 * ggx2;
}

float2 IntegrateBRDF(float NdotV, float roughness)
{
    float3 V;
    V.x = sqrt(1.0f - NdotV*NdotV);
    V.y = 0.0f;
    V.z = NdotV;

    float A = 0.0f;
    float B = 0.0f;

    float3 N = float3(0.0f, 0.0f, 1.0f);

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H  = ImportanceSampleGGX(Xi, N, roughness);
        float3 L  = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0f);
        float NdotH = max(H.z, 0.0f);
        float VdotH = max(dot(V, H), 0.0f);

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return float2(A, B);
}

float4 main(PSIn input) : SV_TARGET
{
    float2 integratedBRDF = IntegrateBRDF(input.uv.x, input.uv.y);
	return float4(integratedBRDF, 0.f, 1.0f);
}