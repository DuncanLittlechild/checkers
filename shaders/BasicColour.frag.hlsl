
cbuffer OncePerLoop : register(b0, space3)
{};
cbuffer OncePerObject : register(b1, space3)
{
    float3 trueColour : packoffset(c0);
};

struct Input
{
    float3 colour : TEXCOORD0;
    float4 Position : SV_Position;
};

float4 main (Input input) : SV_Target0
{
    return float4(trueColour, 1.0f);
}