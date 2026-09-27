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
    float4 lSpacePos : TEXCOORD1;
    float4 worldPos : TEXCOORD2;
    float4 Position : SV_Position;
};

float4 main (Input input) : SV_Target0
{
    float3 lightColour = { 1.0, 1.0, 1.0 };

    // Diffuse lighting, just about how much the surface points towards the light
    float3 norms = normalize(input.norms);
    float3 lightDir = normalize(lightPos - input.worldPos.xyz);
    float lightMag = max(dot(lightDir, norms), 0.0);
    float3 finalColour = (lightMag * trueColour * lightColour) + (0.1 * trueColour);

    // specular lighting, about how much light reflects from the surface into the camera
    
    float3 viewDir = normalize(cameraPos - input.worldPos.xyz);
    float3 halfVector = normalize(lightDir + viewDir);
    float specular = pow(max(dot(norms, halfVector), 0.0f), shinyness);
    finalColour += lightColour * specularColour * specular;

    // compare point's depth to the sampled depth
    // Manually perform the perspective division on the lspacepos
    // The GPU only does this to position, so this must be done to match the depth texture
    float3 finalSpacePos = input.lSpacePos.xyz / input.lSpacePos.w;

    // Texture coordinates are [0,1] while x and y in this case are [-1,1] due to being coordinates
    finalSpacePos.xy = finalSpacePos.xy * 0.5 + 0.5;

    float currentDepth = finalSpacePos.z;
    // This uses the finalSpacePos to index into the texture, and then extract the single channel depth value (.r)
    float textureDepth = Texture.Sample(Sampler, finalSpacePos.xy).r;

    float bias = 0.001;

    // if it is greater than the sampled depth, the point is in shadow
    if (textureDepth + bias < currentDepth)
    {
        finalColour *= 0.1;
    }

    return float4(finalColour, 1.0f);
}