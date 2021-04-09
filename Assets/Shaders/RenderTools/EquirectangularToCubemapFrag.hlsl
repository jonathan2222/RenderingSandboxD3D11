struct PSIn
{
    float4 position : SV_POSITION;
    float3 dir : DIRECTION;
};

Texture2D       equirectangularMap : register(t0);
SamplerState    linearSampler;

float2 SampleSphericalMap(float3 v)
{
    const float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float4 main(PSIn input) : SV_TARGET
{
    float2 uv = SampleSphericalMap(normalize(input.dir));
    float3 textureColor = equirectangularMap.Sample(linearSampler, uv).rgb;
	return float4(textureColor, 1.0f);
}