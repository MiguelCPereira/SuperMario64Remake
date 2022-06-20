float4x4 gWorld;
float4x4 gLightViewProj;
float4x4 gBones[70];
 
DepthStencilState depthStencilState
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState rasterizerState
{
	FillMode = SOLID;
	CullMode = NONE;
};

//--------------------------------------------------------------------------------------
// Vertex Shader [STATIC]
//--------------------------------------------------------------------------------------
float4 ShadowMapVS(float3 position:POSITION):SV_POSITION
{
	//TODO: return the position of the vertex in correct space (hint: seen from the view of the light)
	float4 lightSpaceVertex = mul ( float4(position,1.0f), gWorld );
	//float4 lightSpaceVertex = float4(position,1.0f);
	lightSpaceVertex = mul ( lightSpaceVertex, gLightViewProj );
	return lightSpaceVertex;
}

//--------------------------------------------------------------------------------------
// Vertex Shader [SKINNED]
//--------------------------------------------------------------------------------------
float4 ShadowMapVS_Skinned(float3 position:POSITION, float4 BoneIndices : BLENDINDICES, float4 BoneWeights : BLENDWEIGHTS) : SV_POSITION
{
	//TODO: return the position of the ANIMATED vertex in correct space (hint: seen from the view of the light)
	
	// Claculate the animated vertex final pos
	// ( By adding the position ammount of each bone that affects this vertex)
	float4 transformedPos;
	for (int i = 0; i < 4; i++)
	{
		float currentBoneIdx = BoneIndices[i];
		if (currentBoneIdx >= 0)
		{
			float4 bonePos = mul(float4(position, 1.0f), gBones[currentBoneIdx]);
			transformedPos += bonePos * BoneWeights[i];
		}
	}
	transformedPos[3] = 1;
	
	// Convert it into the correct space
	float4 lightSpaceAnimVertex = mul ( transformedPos, gWorld );
	lightSpaceAnimVertex = mul ( lightSpaceAnimVertex, gLightViewProj );
	
	
	return lightSpaceAnimVertex;
}
 
//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
void ShadowMapPS_VOID(float4 position:SV_POSITION){}

technique11 GenerateShadows
{
	pass P0
	{
		SetRasterizerState(rasterizerState);
	    SetDepthStencilState(depthStencilState, 0);
		SetVertexShader(CompileShader(vs_4_0, ShadowMapVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapPS_VOID()));
	}
}

technique11 GenerateShadows_Skinned
{
	pass P0
	{
		SetRasterizerState(rasterizerState);
		SetDepthStencilState(depthStencilState, 0);
		SetVertexShader(CompileShader(vs_4_0, ShadowMapVS_Skinned()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, ShadowMapPS_VOID()));
	}
}