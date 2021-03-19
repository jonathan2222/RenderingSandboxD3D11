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
    float3 textureColor = skyboxTexture.Sample(texSampler, input.localPos.xyz).rgb;
    //float3 textureColor = skyboxTexture.Sample(texSampler, float3(input.uv.x, input.uv.y, 0.f)).rgb;
	return float4(textureColor, 1.0f);
}