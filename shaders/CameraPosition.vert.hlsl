cbuffer MvpTrans : register(b0, space1)
{
    row_major float4x4 MoveTransform : packoffset(c0);
};
cbuffer VpTrans : register(b1, space1)
{
    row_major float4x4 VpTransform;
    row_major float4x4 LvpTrans;
};

struct Input
{
    float3 position : TEXCOORD0;
    float3 norms : TEXCOORD1;
    float3 colour : TEXCOORD2;
};

struct Output
{
    float3 norms : TEXCOORD0;
    float4 lSpacePos : TEXCOORD1;
    float4 worldPos : TEXCOORD2;
    float4 Position : SV_Position;
};

Output main (Input input)
{
    Output output;
    // Get the point's position in worldspace
    output.worldPos = mul(float4(input.position, 1.0f), MoveTransform);
    // Transform it into clipspace
    output.Position = mul(output.worldPos, VpTransform);

    // Get the vertices position in the lights clipspace
    output.lSpacePos = mul(output.worldPos, LvpTrans);

    // Calculate diffuse lighting
    output.norms = input.norms;

    return output;
}