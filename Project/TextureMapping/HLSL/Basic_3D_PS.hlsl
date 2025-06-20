#include "Basic.hlsli"
 
float4 PS(VertexPosHWNormalTex pIn) : SV_TARGET 
{
	pIn.normalW = normalize(pIn.normalW);
	float3 toEyeW = normalize(g_EyePosW - pIn.posW);

	float4 ambient = float4(0.0f,0.0f,0.0f,0.0f);
	float4 diffuse = float4(0.0f,0.0f,0.0f,0.0f);
	float4 spec = float4(0.0f,0.0f,0.0f,0.0f);
	float4 A = float4(0.0f,0.0f,0.0f,0.0f);
	float4 D = float4(0.0f,0.0f,0.0f,0.0f);
	float4 S = float4(0.0f,0.0f,0.0f,0.0f);

	int i;

	for(i = 0; i < g_NumDirLight; i++) {
		ComputeDirectionLight(g_Material, g_DirLight[i], pIn.normalW, toEyeW, A, D, S);

		ambient += A;
		diffuse += D;
		spec += S;
	}

	for(int i = 0; i< g_NumPointLight; i++) {
		ComputePointLight(g_Material, g_PointLight[i], pIn.normalW, pIn.normalW, toEyeW, A, D, S);

		ambient += A;
		diffuse += D;
		spec += S;
	}

	for(int i = 0; i < g_NumSpotLight; i++) {
		ComputeSpotLight(g_Material, g_SpotLight[i], pIn.posW, pIn.normalW, toEyeW, A, D, S);

		ambient += A;
		diffuse += D;
		spec += S;	
	}	

	float4 texColor = g_Tex.Sample(g_SamLinear, pIn.tex);
	float4 litColor = texColor * (ambient + diffuse) +spec;

	litColor.a = texColor.a * g_Material.diffuse.a;
	return litColor;
}