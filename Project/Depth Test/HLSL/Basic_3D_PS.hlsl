#include "Basic.hlsli"

float4 PS(VertexPosHWNormalTex pIn) : SV_Target
{
	float4 texColor = g_Tex.Sample(g_SamLinear, pIn.tex);
	clip(texColor.a - 0.1f);

	pIn.normalW = normalize(pIn.normalW);

	float3 toEyeW = normalize(g_EyePosW - pIn.posW);

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
	int i;

	[unroll]
	for (i = 0; i < 5; ++i)
	{
		DirectionalLight dirLight = g_DirLight[i];
		[flatten]
		if (g_IsReflection)
		{
			dirLight.direction = mul(dirLight.direction, (float3x3) (g_Reflection));
		}
		ComputeDirectionLight(g_Material, dirLight, pIn.normalW, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}
	PointLight pointLight;
    [unroll]
	for (i = 0; i < 5; ++i)
	{
		pointLight = g_PointLight[i];
        [flatten]
		if (g_IsReflection)
		{
			pointLight.position = (float3) mul(float4(pointLight.position, 1.0f), g_Reflection);
		}
		ComputePointLight(g_Material, pointLight, pIn.posW, pIn.normalW, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}
	
	SpotLight spotLight;
	[unroll]
	for (i = 0; i < 5; ++i)
	{
		spotLight = g_SpotLight[i];
        [flatten]
		if (g_IsReflection)
		{
			spotLight.position = (float3) mul(float4(spotLight.position, 1.0f), g_Reflection);
			spotLight.direction = mul(spotLight.direction, (float3x3) g_Reflection);
		}
		ComputeSpotLight(g_Material, spotLight, pIn.posW, pIn.normalW, toEyeW, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}
    
	float4 litColor = texColor * (ambient + diffuse) + spec;
	litColor.a = texColor.a * g_Material.diffuse.a;
	return litColor;
}
