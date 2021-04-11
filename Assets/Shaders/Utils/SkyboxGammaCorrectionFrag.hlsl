struct PSIn
{
    float4 position : SV_POSITION;
    float4 localPos : POSITION;
    float2 uv : UV;
};

TextureCube     skyboxTexture : register(t0);
SamplerState    texSampler;

float4 main(PSIn input) : SV_TARGET
{
    float3 envColor = skyboxTexture.Sample(texSampler, input.localPos.xyz).rgb;
    float3 oneF3 = float3(1.f, 1.f, 1.f);
    float gammaCorrectionExponent = 1.f/2.2f;
    envColor = envColor / (envColor + oneF3);
    envColor = pow(envColor, float3(gammaCorrectionExponent, gammaCorrectionExponent, gammaCorrectionExponent));

	return float4(envColor, 1.0f);
}