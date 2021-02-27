struct PSIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float4 main(PSIn input) : SV_TARGET
{
    float4 normal = normalize(input.normal);
	return float4(normal.rgb*0.5f+0.5f, 1.0f);
}