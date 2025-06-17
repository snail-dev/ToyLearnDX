#include "Cube.hlsli"

float4 PS(VertexOut pIn) : SV_Target {
     return g_UseCustomColor ? g_CustomColor : pIn.color;
}