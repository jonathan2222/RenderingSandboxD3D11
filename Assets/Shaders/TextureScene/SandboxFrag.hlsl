struct PSIn
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD;
};

Texture2D tex : register(t0);
SamplerState texSampler;

float4 main(PSIn input) : SV_TARGET
{
    float4 textureColor = tex.Sample(texSampler, input.uv);
	return textureColor + float4(input.color.rgb*0.2f, input.color.a);
}