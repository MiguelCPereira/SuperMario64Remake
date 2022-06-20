
#include "stdafx.h"
#include "EffectHelper.h"

const std::wstring EffectHelper::m_empty = L"";
std::map<ILSemantic, std::wstring> EffectHelper::m_IlSemanticName = create_map();

const std::wstring& EffectHelper::GetIlSemanticName(ILSemantic semantic)
{
	auto pair = m_IlSemanticName.find(semantic);
	if (pair != m_IlSemanticName.end())
		return pair->second;
	return m_empty;
}

bool EffectHelper::BuildInputLayout(ID3D11Device* pDevice, ID3DX11EffectTechnique* pTechnique, ID3D11InputLayout** pInputLayout, std::vector<ILDescription>& inputLayoutDescriptions, UINT& inputLayoutSize, UINT& inputLayoutID)
{
	D3DX11_PASS_SHADER_DESC passShaderDesc;
	pTechnique->GetPassByIndex(0)->GetVertexShaderDesc(&passShaderDesc);

	D3DX11_EFFECT_SHADER_DESC effectShaderDesc;
	passShaderDesc.pShaderVariable->GetShaderDesc(passShaderDesc.ShaderIndex, &effectShaderDesc);

	D3D11_SIGNATURE_PARAMETER_DESC signParDesc;
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc;
	inputLayoutSize = 0;

	inputLayoutDescriptions.clear();
	for (UINT i = 0; i < effectShaderDesc.NumInputSignatureEntries; ++i)
	{
		passShaderDesc.pShaderVariable->GetInputSignatureElementDesc(passShaderDesc.ShaderIndex, i, &signParDesc);
		ILDescription ilDescription;
		ZeroMemory(&ilDescription, sizeof(ILDescription));

		//**FILL UP ilDescription

		//Semantic offset

		//MASK LAYOUT
		//0000 0001 => 4 bytes	(Active Bits = 1)
		//0000 0011 => 8 bytes	(Active Bits = 2)
		//0000 0111 => 12 bytes	(Active Bits = 3)
		//0000 1111 => 16 bytes	(Active Bits = 4)
		//OFFSET = 'Active Bits' * 4 [Active Bits = log2(number) + 1]
		ilDescription.Offset = static_cast<UINT>(floor(log(signParDesc.Mask) / log(2)) + 1) * 4;

		//Semantic Format
		switch (signParDesc.ComponentType)
		{
		case D3D10_REGISTER_COMPONENT_FLOAT32:
			if (signParDesc.Mask == 1)ilDescription.Format = DXGI_FORMAT_R32_FLOAT;
			else if (signParDesc.Mask == 3)ilDescription.Format = DXGI_FORMAT_R32G32_FLOAT;
			else if (signParDesc.Mask == 7)ilDescription.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			else ilDescription.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case D3D10_REGISTER_COMPONENT_UINT32:
			if (signParDesc.Mask == 1)ilDescription.Format = DXGI_FORMAT_R32_UINT;
			else if (signParDesc.Mask == 3)ilDescription.Format = DXGI_FORMAT_R32G32_UINT;
			else if (signParDesc.Mask == 7)ilDescription.Format = DXGI_FORMAT_R32G32B32_UINT;
			else ilDescription.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			break;
		case D3D10_REGISTER_COMPONENT_SINT32:
			if (signParDesc.Mask == 1)ilDescription.Format = DXGI_FORMAT_R32_SINT;
			else if (signParDesc.Mask == 3)ilDescription.Format = DXGI_FORMAT_R32G32_SINT;
			else if (signParDesc.Mask == 7)ilDescription.Format = DXGI_FORMAT_R32G32B32_SINT;
			else ilDescription.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			break;
		default:
			HANDLE_ERROR(L"Unsupported Component Type!");
			break;
		}

		//Semantic Type
		if (strcmp(signParDesc.SemanticName, "POSITION") == 0)ilDescription.SemanticType = ILSemantic::POSITION;
		else if (strcmp(signParDesc.SemanticName, "NORMAL") == 0)ilDescription.SemanticType = ILSemantic::NORMAL;
		else if (strcmp(signParDesc.SemanticName, "COLOR") == 0)ilDescription.SemanticType = ILSemantic::COLOR;
		else if (strcmp(signParDesc.SemanticName, "TEXCOORD") == 0)ilDescription.SemanticType = ILSemantic::TEXCOORD;
		else if (strcmp(signParDesc.SemanticName, "TANGENT") == 0)ilDescription.SemanticType = ILSemantic::TANGENT;
		else if (strcmp(signParDesc.SemanticName, "BINORMAL") == 0)ilDescription.SemanticType = ILSemantic::BINORMAL;
		else if (strcmp(signParDesc.SemanticName, "BLENDINDICES") == 0)ilDescription.SemanticType = ILSemantic::BLENDINDICES;
		else if (strcmp(signParDesc.SemanticName, "BLENDWEIGHTS") == 0)ilDescription.SemanticType = ILSemantic::BLENDWEIGHTS;
		else HANDLE_ERROR(L"Semantic Type \"{}\" is not supported!", StringUtil::utf8_decode(signParDesc.SemanticName));

		//Enable corresponding BitFlag
		//inputLayoutID += ilDescription.SemanticType * static_cast<UINT>(pow(10, i));
		inputLayoutID |= static_cast<UINT>(ilDescription.SemanticType);

		//wchar_t buffer[50];
		//wsprintf(buffer,L"This is a test with %d results",5);
		//Logger::Log(std::wstring((wchar_t*)&buffer),LogLevel::Error);
		//Semantic Index
		ilDescription.SemanticIndex = signParDesc.SemanticIndex;

		//Add Description to Vector
		inputLayoutDescriptions.push_back(ilDescription);

		//**Create Input Element Desc
		D3D11_INPUT_ELEMENT_DESC inputLayout = { signParDesc.SemanticName, signParDesc.SemanticIndex, ilDescription.Format, 0, inputLayoutSize, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		layoutDesc.push_back(inputLayout);

		//Increment Position
		inputLayoutSize += ilDescription.Offset;
	}

	// Create the input layout
	D3DX11_PASS_DESC PassDesc;
	// Get the pass decriptor from the effect technique
	pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	HANDLE_ERROR(pDevice->CreateInputLayout(&layoutDesc[0], static_cast<UINT>(layoutDesc.size()), PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, pInputLayout));

	return true;
}

bool EffectHelper::BuildInputLayout(ID3D11Device* pDevice, ID3DX11EffectTechnique* pTechnique, ID3D11InputLayout** pInputLayout)
{
	UINT layoutSize{};
	return BuildInputLayout(pDevice, pTechnique, pInputLayout, layoutSize);
}

bool EffectHelper::BuildInputLayout(ID3D11Device* pDevice, ID3DX11EffectTechnique* pTechnique, ID3D11InputLayout** pInputLayout, UINT& inputLayoutSize)
{
	D3DX11_PASS_SHADER_DESC passShaderDesc;
	pTechnique->GetPassByIndex(0)->GetVertexShaderDesc(&passShaderDesc);

	D3DX11_EFFECT_SHADER_DESC effectShaderDesc;
	passShaderDesc.pShaderVariable->GetShaderDesc(passShaderDesc.ShaderIndex, &effectShaderDesc);

	D3D11_SIGNATURE_PARAMETER_DESC signParDesc;
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc;
	inputLayoutSize = 0;

	for (UINT i = 0; i < effectShaderDesc.NumInputSignatureEntries; ++i)
	{
		passShaderDesc.pShaderVariable->GetInputSignatureElementDesc(passShaderDesc.ShaderIndex, i, &signParDesc);
#pragma warning(push)
#pragma warning(disable: 26812)
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		UINT offset{};
#pragma warning(pop)

		//**FILL UP ilDescription

		//Semantic offset

		//MASK LAYOUT
		//0000 0001 => 4 bytes	(Active Bits = 1)
		//0000 0011 => 8 bytes	(Active Bits = 2)
		//0000 0111 => 12 bytes	(Active Bits = 3)
		//0000 1111 => 16 bytes	(Active Bits = 4)
		//OFFSET = 'Active Bits' * 4 [Active Bits = log2(number) + 1]
		offset = static_cast<UINT>(floor(log(signParDesc.Mask) / log(2)) + 1) * 4;

		//Semantic Format
		switch (signParDesc.ComponentType)
		{
		case D3D10_REGISTER_COMPONENT_FLOAT32:
			if (signParDesc.Mask == 1)format = DXGI_FORMAT_R32_FLOAT;
			else if (signParDesc.Mask == 3)format = DXGI_FORMAT_R32G32_FLOAT;
			else if (signParDesc.Mask == 7)format = DXGI_FORMAT_R32G32B32_FLOAT;
			else format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case D3D10_REGISTER_COMPONENT_UINT32:
			if (signParDesc.Mask == 1)format = DXGI_FORMAT_R32_UINT;
			else if (signParDesc.Mask == 3)format = DXGI_FORMAT_R32G32_UINT;
			else if (signParDesc.Mask == 7)format = DXGI_FORMAT_R32G32B32_UINT;
			else format = DXGI_FORMAT_R32G32B32A32_UINT;
			break;
		case D3D10_REGISTER_COMPONENT_SINT32:
			if (signParDesc.Mask == 1)format = DXGI_FORMAT_R32_SINT;
			else if (signParDesc.Mask == 3)format = DXGI_FORMAT_R32G32_SINT;
			else if (signParDesc.Mask == 7)format = DXGI_FORMAT_R32G32B32_SINT;
			else format = DXGI_FORMAT_R32G32B32A32_SINT;
			break;
		default:
			HANDLE_ERROR(L"Unsupported Component Type!");
			break;
		}

		//**Create Input Element Desc
		D3D11_INPUT_ELEMENT_DESC inputLayout = { signParDesc.SemanticName, signParDesc.SemanticIndex, format, 0, inputLayoutSize, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		layoutDesc.push_back(inputLayout);

		//Increment Offset
		inputLayoutSize += offset;
	}

	// Create the input layout
	D3DX11_PASS_DESC PassDesc;
	// Get the pass decriptor from the effect technique
	pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	HANDLE_ERROR(pDevice->CreateInputLayout(&layoutDesc[0], static_cast<UINT>(layoutDesc.size()), PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, pInputLayout))

	return true; //redundant return...
}