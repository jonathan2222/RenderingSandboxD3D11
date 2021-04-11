struct PSIn
{
    float4 position : SV_POSITION;
    float3 dir : DIRECTION;
};

TextureCube     environmentMap : register(t0);
SamplerState    linearSampler;

float4 main(PSIn input) : SV_TARGET
{
    const float PI = 3.14159265359f;

    float3 normal = normalize(input.dir);
    float3 irradiance = float3(0.f, 0.f, 0.f);  

    float3 up       = float3(0.f, 1.f, 0.f);
    float3 right    = normalize(cross(up, normal));
    up              = normalize(cross(normal, right));

    float sampleDelta = 0.025f;
    float nrSamples = 0.f; 
    for(float phi = 0.f; phi < 2.f * PI; phi += sampleDelta)
    {
        for(float theta = 0.f; theta < 0.5f * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

            irradiance += environmentMap.Sample(linearSampler, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.f / nrSamples);

	return float4(irradiance, 1.0f);
}