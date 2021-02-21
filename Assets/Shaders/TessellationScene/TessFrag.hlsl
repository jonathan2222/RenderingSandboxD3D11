struct PSIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

Texture2D       albedoTexture : register(t0);
SamplerState    texSampler;

float4 main(PSIn input) : SV_TARGET
{
    float4 textureColor = albedoTexture.Sample(texSampler, input.uv);
	return float4(textureColor.rgb, 1.0f);
}