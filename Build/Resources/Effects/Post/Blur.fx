//=============================================================================
//// Shader uses position and texture
//=============================================================================
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
	float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	
	// Step 1: find the dimensions of the texture (the texture has a method for that)
	float width;
	float height;
	gTexture.GetDimensions(width, height);
	
	// Step 2: calculate dx and dy (UV space for 1 pixel)	
	float dX = 1 / width;
	float dY = 1 / height;
	
	// Step 3: Create a double for loop (5 iterations each)
	for(int i = 0; i < 5; i++)
	{
		for(int e = 0; e < 5; e++)
		{
			// Inside the loop, calculate the offset in each direction. Make sure not to take every pixel but move by 2 pixels each time
			// Do a texture lookup using your previously calculated uv coordinates + the offset, and add to the final color
			float2 uvCoord = input.TexCoord + float2( (i-2)*dX, (e-2)*dY);
			finalColor += gTexture.Sample(samPoint, uvCoord);
		}
	}
	
	// Step 4: Divide the final color by the number of passes (in this case 5*5)
	finalColor /= 5*5;
	


	// Step 5a: calculate the intermidiate value between the original and the given color
	float4 sample = gTexture.Sample(samPoint, input.TexCoord);
	float leveledX = sample.x * (1.0f - gIntensity) + finalColor.x * gIntensity;
	float leveledY = sample.y * (1.0f - gIntensity) + finalColor.y * gIntensity;
	float leveledZ = sample.z * (1.0f - gIntensity) + finalColor.z * gIntensity;

	// Step 5b: return the color
	return float4(leveledX, leveledY, leveledZ, 1.0f);


	//return finalColor;
}


//TECHNIQUE
//---------
technique11 Blur
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