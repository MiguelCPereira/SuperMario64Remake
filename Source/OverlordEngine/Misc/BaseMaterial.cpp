#include "stdafx.h"
#include "BaseMaterial.h"
#include <ranges>

std::map<std::string, BaseMaterial::eRootVariable> BaseMaterial::m_RootVariableSemanticLUT = {
		{"world", eRootVariable::WORLD},
		{"view", eRootVariable::VIEW},
		{"viewinverse", eRootVariable::VIEW_INVERSE},
		{"worldviewprojection", eRootVariable::WORLD_VIEW_PROJECTION}
};

BaseMaterial::~BaseMaterial()
{
	SafeRelease(m_pEffect);
}

void BaseMaterial::_baseInitialize(ID3DX11Effect* pRootEffect, UINT materialId)
{
	if (m_IsInitialized) return;

	m_MaterialId = materialId;
	pRootEffect->CloneEffect(0, &m_pEffect);

	//Update Technique Pointers
	const auto& techniqueCtxs = GetTechniques();
	for(UINT techIndex{0}; techIndex < m_numTechniques; ++techIndex)
	{
		auto it = techniqueCtxs.begin();
		std::advance(it, techIndex);
		auto& ctx = const_cast<MaterialTechniqueContext&>(it->second);
		ctx.pTechnique = m_pEffect->GetTechniqueByIndex(techIndex);
	}

	//Retrieve Root Variables
	for(UINT i{0}; i < static_cast<UINT>(eRootVariable::COUNT); ++i)
	{
		auto rootVariableIndex = GetRootVariableIndex(static_cast<eRootVariable>(i));
		if(rootVariableIndex >= 0)
		{
			m_RootVariableLUT[i] = m_pEffect->GetVariableByIndex(static_cast<UINT>(rootVariableIndex));
		}
	}

	m_IsInitialized = true;
}

void BaseMaterial::UpdateEffectVariables(const SceneContext& sceneContext, const ModelComponent* pModelComponent)
{
	if (!NeedsUpdate(sceneContext.frameNumber, pModelComponent->GetComponentId())) return;

	if (m_IsInitialized)
	{
		m_LastUpdateFrame = sceneContext.frameNumber;
		m_LastUpdateID = pModelComponent->GetComponentId();

		//Update Root Variables
		auto world = XMLoadFloat4x4(&pModelComponent->GetTransform()->GetWorld());
		auto view = XMLoadFloat4x4(&sceneContext.pCamera->GetView());

		if (m_RootVariableLUT[static_cast<UINT>(eRootVariable::WORLD)])
			m_RootVariableLUT[static_cast<UINT>(eRootVariable::WORLD)]->AsMatrix()->SetMatrix(reinterpret_cast<float*>(&world));

		if (m_RootVariableLUT[static_cast<UINT>(eRootVariable::VIEW)])
			m_RootVariableLUT[static_cast<UINT>(eRootVariable::VIEW)]->AsMatrix()->SetMatrix(reinterpret_cast<float*>(&view));

		if (m_RootVariableLUT[static_cast<UINT>(eRootVariable::WORLD_VIEW_PROJECTION)])
		{
			const auto projection = XMLoadFloat4x4(&sceneContext.pCamera->GetProjection());
			auto wvp = world * view * projection;
			m_RootVariableLUT[static_cast<UINT>(eRootVariable::WORLD_VIEW_PROJECTION)]->AsMatrix()->SetMatrix(reinterpret_cast<float*>(&wvp));
		}

		if (m_RootVariableLUT[static_cast<UINT>(eRootVariable::VIEW_INVERSE)])
		{
			auto& viewInv = sceneContext.pCamera->GetViewInverse();
			m_RootVariableLUT[static_cast<UINT>(eRootVariable::VIEW_INVERSE)]->AsMatrix()->SetMatrix(&viewInv._11);
		}

		OnUpdateModelVariables(sceneContext, pModelComponent);
	}
}

bool BaseMaterial::NeedsUpdate(UINT frame, UINT id) const
{
	if (m_LastUpdateFrame == 0 && m_LastUpdateID == 0) return true;
	return m_LastUpdateFrame != frame || m_LastUpdateID != id;
}

ID3DX11EffectVariable* BaseMaterial::GetVariable(const std::wstring& varName) const
{
	auto& variableLUT = GetVariableIndexLUT();
	const auto variableHash = std::hash<std::wstring>{}(varName);

	if (!variableLUT.contains(variableHash)) return nullptr;
	return m_pEffect->GetVariableByIndex(variableLUT.at(variableHash));
}

void BaseMaterial::SetVariable_Scalar(const std::wstring& varName, float scalar) const
{
	if(const auto pShaderVariable = GetVariable(varName))
	{
		HANDLE_ERROR(pShaderVariable->AsScalar()->SetFloat(scalar));
		return;
	}

	Logger::LogWarning(L"Shader variable \'{}\' not found for \'{}\'", varName, GetEffectName());
}

void BaseMaterial::SetVariable_Scalar(const std::wstring& varName, bool scalar) const
{
	if (const auto pShaderVariable = GetVariable(varName))
	{
		HANDLE_ERROR(pShaderVariable->AsScalar()->SetBool(scalar));
		return;
	}

	Logger::LogWarning(L"Shader variable \'{}\' not found for \'{}\'", varName, GetEffectName());
}
void BaseMaterial::SetVariable_Scalar(const std::wstring& varName, int scalar) const
{
	if (const auto pShaderVariable = GetVariable(varName))
	{
		HANDLE_ERROR(pShaderVariable->AsScalar()->SetInt(scalar));
		return;
	}

	Logger::LogWarning(L"Shader variable \'{}\' not found for \'{}\'", varName, GetEffectName());
}

void BaseMaterial::SetVariable_Matrix(const std::wstring& varName, XMFLOAT4X4 matrix) const
{
	SetVariable_Matrix(varName, &matrix._11);
}

void BaseMaterial::SetVariable_Matrix(const std::wstring& varName, const float* pData) const
{
	if (const auto pShaderVariable = GetVariable(varName))
	{
		HANDLE_ERROR(pShaderVariable->AsMatrix()->SetMatrix(pData));
		return;
	}

	Logger::LogWarning(L"Shader variable \'{}\' not found for \'{}\'", varName, GetEffectName());
}

void BaseMaterial::SetVariable_MatrixArray(const std::wstring& varName, const float* pData, UINT count) const
{
	if (const auto pShaderVariable = GetVariable(varName))
	{
		HANDLE_ERROR(pShaderVariable->AsMatrix()->SetMatrixArray(pData, 0, count));
		return;
	}

	Logger::LogWarning(L"Shader variable \'{}\' not found for \'{}\'", varName, GetEffectName());
}

void BaseMaterial::SetVariable_Vector(const std::wstring& varName, XMFLOAT3 vector) const
{
	SetVariable_Vector(varName, &vector.x);
}

void BaseMaterial::SetVariable_Vector(const std::wstring& varName, XMFLOAT4 vector) const
{
	SetVariable_Vector(varName, &vector.x);
}

void BaseMaterial::SetVariable_Vector(const std::wstring& varName, const float* pData) const
{
	if (const auto pShaderVariable = GetVariable(varName))
	{
		HANDLE_ERROR(pShaderVariable->AsVector()->SetFloatVector(pData));
		return;
	}

	Logger::LogWarning(L"Shader variable \'{}\' not found for \'{}\'", varName, GetEffectName());
}

void BaseMaterial::SetVariable_VectorArray(const std::wstring& varName, const float* pData, UINT count) const
{
	if (const auto pShaderVariable = GetVariable(varName))
	{
		HANDLE_ERROR(pShaderVariable->AsVector()->SetFloatVectorArray(pData, 0, count));
		return;
	}

	Logger::LogWarning(L"Shader variable \'{}\' not found for \'{}\'", varName, GetEffectName());
}

void BaseMaterial::SetVariable_Texture(const std::wstring& varName, ID3D11ShaderResourceView* pSRV) const
{
	if (const auto pShaderVariable = GetVariable(varName))
	{
		HANDLE_ERROR(pShaderVariable->AsShaderResource()->SetResource(pSRV));
		return;
	}

	Logger::LogWarning(L"Shader variable \'{}\' not found for \'{}\'", varName, GetEffectName());
}

void BaseMaterial::SetVariable_Texture(const std::wstring& varName, const TextureData* pTexture) const
{
	SetVariable_Texture(varName, pTexture->GetShaderResourceView());
}

void BaseMaterial::SetTechnique(const std::wstring& techName)
{
	auto& techniques = GetTechniques();
	const auto techNameHash = std::hash<std::wstring>{}(techName);

	if(techniques.contains(techNameHash))
	{
		m_TechniqueContext = techniques.at(techNameHash);
		m_TechniqueContext.pTechnique = m_pEffect->GetTechniqueByName(StringUtil::utf8_encode(techName).c_str());
		return;
	}

	Logger::LogWarning(L"Shader technique \'{}\' not found for \'{}\'", techName, GetEffectName());
}

void BaseMaterial::SetTechnique(int index)
{
	auto& techniques = GetTechniques();
	if(techniques.size() > index)
	{
		auto it = techniques.begin();
		std::advance(it, index);
		m_TechniqueContext = it->second;
		m_TechniqueContext.pTechnique = m_pEffect->GetTechniqueByIndex(index);
		return;
	}

	Logger::LogWarning(L"Shader technique with index \'{}\' not found for \'{}\'", index, GetEffectName());
}

const MaterialTechniqueContext& BaseMaterial::GetTechniqueContext(int index) const
{
	auto& techniques = GetTechniques();
	ASSERT_IF_(techniques.size() <= index);

	auto it = techniques.begin();
	std::advance(it, index);

	return it->second;
}

void BaseMaterial::DrawImGui()
{
	ImGui::Dummy({ 0,5.f });
	const std::string title = std::format("Material > {} (ID#{})", m_MaterialNameUtf8.substr(6), m_MaterialId).c_str();
	if(ImGui::Button(title.c_str())){m_DrawImGui = true;}

	if (!m_DrawImGui) return;

	if (ImGui::Begin(title.c_str(), &m_DrawImGui))
	{
		D3DX11_EFFECT_DESC effectDesc{};
		m_pEffect->GetDesc(&effectDesc);

		for (UINT i{ 0 }; i < effectDesc.GlobalVariables; ++i)
		{
			const auto pVariable = m_pEffect->GetVariableByIndex(i);

			D3DX11_EFFECT_TYPE_DESC effectTypeDesc{};
			pVariable->GetType()->GetDesc(&effectTypeDesc);

			D3DX11_EFFECT_VARIABLE_DESC variableDesc{};
			pVariable->GetDesc(&variableDesc);

			bool isColor = std::string(variableDesc.Name).find("Color") != std::string::npos;

			switch (effectTypeDesc.Type)
			{
			case D3D_SVT_BOOL:
			{
				bool value{};
				pVariable->AsScalar()->GetBool(&value);
				if (ImGui::Checkbox(variableDesc.Name, &value))
				{
					pVariable->AsScalar()->SetBool(value);
				}
			}
			break;
			case D3D_SVT_INT:
			{
				int value{};
				pVariable->AsScalar()->GetInt(&value);
				if (ImGui::DragInt(variableDesc.Name, &value, 0.1f))
				{
					pVariable->AsScalar()->SetInt(value);
				}
			}
			break;
			case D3D_SVT_FLOAT:
			{
				if (effectTypeDesc.Class == D3D_SVC_SCALAR)
				{
					float value{};
					pVariable->AsScalar()->GetFloat(&value);
					if (ImGui::DragFloat(variableDesc.Name, &value, 0.1f))
						pVariable->AsScalar()->SetFloat(value);
				}
				else if (effectTypeDesc.Class == D3D_SVC_VECTOR)
				{
					float value[4]{};
					bool changed{};
					pVariable->AsVector()->GetFloatVector(&value[0]);
					switch (effectTypeDesc.Columns)
					{
					case 2: changed = ImGui::DragFloat2(variableDesc.Name, &value[0], 0.1f); break;
					case 3: changed = isColor ? ImGui::ColorEdit3(variableDesc.Name, &value[0], ImGuiColorEditFlags_NoInputs) : ImGui::DragFloat3(variableDesc.Name, &value[0], 0.1f); break;
					case 4: changed = isColor ? ImGui::ColorEdit4(variableDesc.Name, &value[0], ImGuiColorEditFlags_NoInputs) : ImGui::DragFloat3(variableDesc.Name, &value[0], 0.1f); break;
					}

					if (changed) pVariable->AsVector()->SetFloatVector(&value[0]);
				}
			}
			break;
			case D3D_SVT_TEXTURE2D:
			case D3D_SVT_TEXTURECUBE:
			{
				char buffer[256]{};

				ID3D11ShaderResourceView* pSrv{};
				pVariable->AsShaderResource()->GetResource(&pSrv);

				if (pSrv)
				{
					TextureData* pTexData{};
					UINT size = sizeof(pTexData);
					pSrv->GetPrivateData(TextureData::GUID_TextureData, &size, &pTexData);
					if (pTexData)
					{
						auto pathUtf8 = StringUtil::utf8_encode(pTexData->GetAssetSubPath());
						pathUtf8.copy(&buffer[0], pathUtf8.size());
					}

					pSrv->Release();
				}

				if (ImGui::InputText(variableDesc.Name, &buffer[0], 256, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					auto filepath = StringUtil::utf8_decode(buffer);
					if (fs::exists(ContentManager::GetFullAssetPath(filepath)))
					{
						const auto pTextureData = ContentManager::Load<TextureData>(filepath);
						pVariable->AsShaderResource()->SetResource(pTextureData->GetShaderResourceView());
					}
				}
			}
			break;
			//case D3D_SVT_UINT: break;
			//case D3D_SVT_UINT8: break;
			//case D3D_SVT_DOUBLE: break;
			//default: ;
			}

		}
	}
	ImGui::End();
}
