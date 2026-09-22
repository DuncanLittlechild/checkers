cbuffer MvpTrans : register(b0, space1)
{
    row_major float4x4 MoveTransform : packoffset(c0);
};
cbuffer VpTrans : register(b1, space1)
{
    row_major float4x4 VpTransform : packoffset(c0);
};

struct Input
{
    float3 position : TEXCOORD0;
    float3 norms : TEXCOORD1;
    float3 colour : TEXCOORD2;
};

struct Output
{
    float4 Position : SV_Position;
    float3 colour : TEXCOORD1;
};

Output main (Input input)
{
    Output output;
    output.Position = mul(float4(input.position, 1.0f), mul(MoveTransform, VpTransform));
    return output;
}