#pragma once

	template<class T>
	class Material: public BaseMaterial

{
public:
	Material(const std::wstring& effectFile)
	{
		if(!m_EffectInstanceLoaded)
			m_EffectFile = effectFile;

		++m_References;
	}

	~Material() override
	{
		--m_References;
		if(m_References <= 0)
		{
			m_VariableIndexLUT.clear();

			for(auto& pair:m_Techniques)
			{
				//SafeRelease(pair.second.pTechnique);
				SafeRelease(pair.second.pInputLayout);
				pair.second.inputLayoutID = 0;
				pair.second.inputLayoutSize = 0;
				pair.second.pInputLayoutDescriptions.clear();
			}

			m_Techniques.clear();

			m_EffectInstanceLoaded = false;
		}
	}

	Material(const Material& other) = delete;
	Material(Material&& other) noexcept = delete;
	Material& operator=(const Material& other) = delete;
	Material& operator=(Material&& other) noexcept = delete;

	void Initialize(const D3D11Context& d3d11Context, UINT materialId) override
	{
		if (!m_EffectInstanceLoaded)
		{
			//Load Effect
			m_pRootEffect = ContentManager::Load<ID3DX11Effect>(m_EffectFile);

			//EFFECT VARIABLES
			//(Load)
			D3DX11_EFFECT_DESC effectDesc{};
			m_pRootEffect->GetDesc(&effectDesc);

			m_VariableIndexLUT.clear();
			std::fill_n(m_RootVariableIndexLUT, static_cast<UINT>(eRootVariable::COUNT), -1);

			for (UINT i{ 0 }; i < effectDesc.GlobalVariables; ++i)
			{
				const auto pVariable = m_pRootEffect->GetVariableByIndex(i);
				D3DX11_EFFECT_VARIABLE_DESC variableDesc{};
				pVariable->GetDesc(&variableDesc);

				auto variableHash = std::hash < std::wstring >{}(StringUtil::utf8_decode(variableDesc.Name));
				m_VariableIndexLUT.insert(std::make_pair(variableHash, i));

				//Search Root Variable
				//Check for Semantic >> Possible Root Variable
				if (variableDesc.Semantic == nullptr) continue;

				auto semanticStr = std::string(variableDesc.Semantic);
				std::ranges::transform(semanticStr, semanticStr.begin(), ::tolower);
				if(m_RootVariableSemanticLUT.contains(semanticStr))
				{
					m_RootVariableIndexLUT[static_cast<UINT>(m_RootVariableSemanticLUT[semanticStr])] = i;
				}
			}

			//TECHNIQUES
			//(Load)
			m_numTechniques = effectDesc.Techniques;
			m_Techniques.clear();

			for (UINT i{ 0 }; i < m_numTechniques; ++i)
			{
				const auto pTechnique = m_pRootEffect->GetTechniqueByIndex(i);
				D3DX11_TECHNIQUE_DESC techDesc{};
				pTechnique->GetDesc(&techDesc);

				//Create Technique Context
				MaterialTechniqueContext techCtx{};
				techCtx.pTechnique = pTechnique;

				EffectHelper::BuildInputLayout(d3d11Context.pDevice, pTechnique, &techCtx.pInputLayout, techCtx.pInputLayoutDescriptions, techCtx.inputLayoutSize, techCtx.inputLayoutID);

				//Add to map
				auto techniqueHash = std::hash < std::wstring >{}(StringUtil::utf8_decode(techDesc.Name));
				m_Techniques.insert(std::make_pair(techniqueHash, techCtx));
			}

			m_EffectInstanceLoaded = true;
		}

		_baseInitialize(m_pRootEffect, materialId);
		SetTechnique(0);

		InitializeEffectVariables();
	}

protected:
	const std::map<size_t, UINT>& GetVariableIndexLUT() const override { return m_VariableIndexLUT; }
	int GetRootVariableIndex(eRootVariable rootVariable) const override { return m_RootVariableIndexLUT[static_cast<size_t>(rootVariable)]; }
	const std::map<size_t, MaterialTechniqueContext>& GetTechniques() const override { return m_Techniques; }
	const std::wstring& GetEffectName() const override { return m_EffectFile; }

private:
	static int m_References;
	static bool m_EffectInstanceLoaded;
	static std::map<size_t, UINT> m_VariableIndexLUT;
	static std::map<size_t, MaterialTechniqueContext> m_Techniques;
	static int m_RootVariableIndexLUT[];

	static ID3DX11Effect* m_pRootEffect;
	static std::wstring m_EffectFile;
};

template<class T> bool Material<T>::m_EffectInstanceLoaded{};
template<class T> int Material<T>::m_References{0};
template<class T> std::wstring Material<T>::m_EffectFile{};
template<class T> ID3DX11Effect* Material<T>::m_pRootEffect{};
template<class T> std::map<size_t, UINT> Material<T>::m_VariableIndexLUT{};
template<class T> int Material<T>::m_RootVariableIndexLUT[static_cast<UINT>(eRootVariable::COUNT)]{};
template<class T> std::map<size_t, MaterialTechniqueContext> Material<T>::m_Techniques{};
