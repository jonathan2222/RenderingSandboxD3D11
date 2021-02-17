struct PSIn
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float4 main(PSIn input) : SV_TARGET
{
	return float4(input.normal.rgb*0.5f+0.5f, 1.0f);
}