struct PSIn
{
    float4 position : SV_POSITION;
    float2 uv : UV;
};

Texture2D       oldTexture : register(t0);
SamplerState    texSampler;

float4 main(PSIn input) : SV_TARGET
{
    float3 textureColor = oldTexture.Sample(texSampler, input.uv).rgb;
	return float4(textureColor, 1.0f);
}