cbuffer MvpTrans : register(b0, space1)
{
    row_major float4x4 MoveTransform : packoffset(c0);
};
cbuffer VpTrans : register(b1, space1)
{
    row_major float4x4 VpTransform : packoffset(c0);
};
/*
cbuffer LVpTrans : register(b3, space1)
{
    row_major float4x4 lvpTrans : packoffset(c0);
};*/

struct Input
{
    float3 position : TEXCOORD0;
    float3 norms : TEXCOORD1;
    float3 colour : TEXCOORD2;
};

struct Output
{
    float3 norms : TEXCOORD0;
    float4 worldPos : TEXCOORD1;
    float4 Position : SV_Position;
};

Output main (Input input)
{
    Output output;
    // Get the point's position in worldspace
    output.worldPos = mul(float4(input.position, 1.0f), MoveTransform);
    // Transform it into clipspace
    output.Position = mul(output.worldPos, VpTransform);

    // Calculate diffuse lighting
    output.norms = input.norms;

    return output;
}