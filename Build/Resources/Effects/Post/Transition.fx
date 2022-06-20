//=============================================================================
//// Shader uses position and texture
//=============================================================================

float gCircleRadius = 1.f; // From 0.0f to 1.0f
float3 gBackgroundColor = float3(0.f, 0.f, 0.f);
float2 gCircleCenterPos = float2(0.5f, 0.5f);
float gScreenWidth = 1280.0f;
float gScreenHeight = 720.0f;

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gTexture;

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState DepthWriting
{
	DepthEnable = true;
	DepthWriteMask = 1; //DEPTH_WRITE_MASK_ALL
	DepthFunc = 2; //COMPARISON_LESS
};

/// Create Rasterizer State (Backface culling) 
RasterizerState BackCulling 
{ 
	CullMode = BACK; 
};


//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
    float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	// Set the Position
	output.Position = float4(input.Position, 1.0f);
	// Set the TexCoord
	output.TexCoord = input.TexCoord;
	
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input): SV_Target
{
    // Sample the texture
	float4 sample = gTexture.Sample(samPoint, input.TexCoord);
	
	// Check if point is inside the circle radius and return the sample if so
	float dist = distance(gCircleCenterPos, float2(input.TexCoord.x,
		input.TexCoord.y * (gScreenHeight/gScreenWidth) + gCircleCenterPos.y / 2.0f));
	if (dist < gCircleRadius)
	{
		return float4(sample.x, sample.y, sample.z, 1.0f);
	}

	// If it's outside the radius, return the background color
	return float4(gBackgroundColor.x, gBackgroundColor.y, gBackgroundColor.z, 1.0f);
}


//TECHNIQUE
//---------
technique11 Grayscale
{
    pass P0
    {          
        SetRasterizerState(BackCulling);
		SetDepthStencilState(DepthWriting,0);
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

