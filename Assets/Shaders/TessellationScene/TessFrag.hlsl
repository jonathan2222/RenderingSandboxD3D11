struct PSIn
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float4 normal : NORMAL;
    float3x3 TBN : TBN_MATRIX;
    float2 uv : TEXCOORD;
};

cbuffer FrameData : register(b0)
{
    float4 cameraPos;
    float4 cameraDir;
    float4 lightPos;
}

Texture2D       normalTexture : register(t0);
Texture2D       albedoTexture : register(t1);
SamplerState    texSampler;

float4 main(PSIn input) : SV_TARGET
{
    float3 lightDir = normalize(input.worldPosition.xyz - lightPos.xyz);//normalize(float4(0.0f, -2.f, 0.1f, 0.f));
    input.normal = normalize(input.normal);

    float3 normal = normalTexture.SampleLevel(texSampler, input.uv, 0).xyz;
    normal = normalize(normal*2.f - 1.f);
    normal = mul(input.TBN, normal);

    float3 textureColor = albedoTexture.Sample(texSampler, input.uv).rgb;

    float3 color = float3(0.f, 0.f, 0.f);
    float ambient = 0.1f;
    color += ambient*textureColor.rgb; // Ambient

    float diffuse = max(0.f, dot(-lightDir, normal));
    color += diffuse*textureColor.rgb; // Diffuse

    float3 h = reflect(lightDir, normal);
    float specular = pow(max(0.f, dot(h, -cameraDir.xyz)), 80.f); // Specular
    color += specular*textureColor.rgb;
	return float4(color, 1.0f);
}