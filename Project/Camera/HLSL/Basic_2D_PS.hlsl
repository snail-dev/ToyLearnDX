#include "Basic.hlsli"

float4 PS(VertexPosHTex pIn) : SV_TARGET {
  return g_Tex.Sample(g_SamLinear, pIn.tex);
}
