Texture2D<float> Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

cbuffer OncePerLoop : register(b0, space3)
{
    float3 cameraPos;
    float3 lightPos;
};

cbuffer OncePerObject : register(b1, space3)
{
    float3 trueColour;
    float3 specularColour;
    float shinyness;
};

/*
cbuffer LightMatrix : register(b3, space3)
{
    row_major float4x4 lightVP : packoffset(c0);
}*/

struct Input
{
    float3 norms : TEXCOORD0;
    float4 worldPos : TEXCOORD1;
    float4 Position : SV_Position;
};

float4 main (Input input) : SV_Target0
{
    float3 lightColour = { 1.0, 1.0, 1.0 };
    // output.halfVector = normalize(lightDirection + viewDirection);
    // output.LightMag =
    // Diffuse lighting
    float3 norms = normalize(input.norms);
    float3 lightDir = normalize(lightPos - input.worldPos.xyz);
    float lightMag = max(dot(lightDir, norms), 0.0);
    float3 finalColour = (lightMag * trueColour * lightColour) + (0.1 * trueColour);

    // specular lighting
    
    float3 viewDir = normalize(cameraPos - input.worldPos.xyz);
    float3 halfVector = normalize(lightDir + viewDir);
    float specular = pow(max(dot(norms, halfVector), 0.0f), shinyness);
    finalColour += lightColour * specularColour * specular;
    
    // compare point's depth to the sampled depth
    /*
    float pDepth = Texture.Sample(Sampler, float2(input.lightPosition.x, input.lightPosition.y));
    float bias = 0.001;

    // if it is greater than the sampled depth, the point is in shadow
    if (pDepth > input.lightPosition.z)
    {
        finalColour *= 0.1;
    }*/

    return float4(finalColour, 1.0f);
}