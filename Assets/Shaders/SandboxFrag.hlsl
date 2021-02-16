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
	return textureColor * input.color; //float4(textureColor.x*input.color.x, textureColor.y*input.color.y, textureColor.z*input.color.z, textureColor.w*input.color.w);
}