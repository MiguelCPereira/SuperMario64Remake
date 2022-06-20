//=============================================================================
//// Shader uses position and texture
//=============================================================================

float3 gColor = float3(0.0f, 0.0f, 0.0f);
float gIntensity = 0.5f;

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
    // Step 1: sample the texture
	float4 sample = gTexture.Sample(samPoint, input.TexCoord);
	
	// Step 2: calculate the intermidiate value between the original and the given color
	float coloredX = sample.x * (1.0f - gIntensity) + gColor.x * gIntensity;
	float coloredY = sample.y * (1.0f - gIntensity) + gColor.y * gIntensity;
	float coloredZ = sample.z * (1.0f - gIntensity) + gColor.z * gIntensity;
	
	// Step 3: return the color
    return float4(coloredX, coloredY, coloredZ, 1.0f );
}


//TECHNIQUE
//---------
technique11 Grayscale
{
    pass P0
    {          
        // Set states...
		SetRasterizerState(BackCulling);
		SetDepthStencilState(DepthWriting,0);
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

