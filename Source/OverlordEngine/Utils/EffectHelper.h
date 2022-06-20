#pragma once
//Input Layout Description
enum class ILSemantic : UINT
{
	NONE = 0,
	POSITION = (1 << 0),
	NORMAL = (1 << 1),
	COLOR = (1 << 2),
	TEXCOORD = (1 << 3),
	BINORMAL = (1 << 4),
	TANGENT = (1 << 5),
	BLENDINDICES = (1 << 6),
	BLENDWEIGHTS = (1 << 7)
};
ENABLE_BITMASK_OPERATORS(ILSemantic)

struct ILDescription
{
	ILSemantic SemanticType;
	DXGI_FORMAT Format;
	UINT SemanticIndex;
	UINT Offset;
};

struct MaterialTechniqueContext
{
	ID3DX11EffectTechnique* pTechnique{};
	ID3D11InputLayout* pInputLayout{};
	std::vector<ILDescription> pInputLayoutDescriptions{};
	UINT inputLayoutSize{};
	UINT inputLayoutID{};
};

class EffectHelper final
{
public:
	EffectHelper() = default;
	~EffectHelper() = default;
	EffectHelper(const EffectHelper& other) = delete;
	EffectHelper(EffectHelper&& other) noexcept = delete;
	EffectHelper& operator=(const EffectHelper& other) = delete;
	EffectHelper& operator=(EffectHelper&& other) noexcept = delete;
	static bool BuildInputLayout(ID3D11Device* pDevice, ID3DX11EffectTechnique* pTechnique, ID3D11InputLayout** pInputLayout, std::vector<ILDescription>& inputLayoutDescriptions, UINT& inputLayoutSize, UINT& inputLayoutID);
	static bool BuildInputLayout(ID3D11Device* pDevice, ID3DX11EffectTechnique* pTechnique, ID3D11InputLayout** pInputLayout, UINT& inputLayoutSize);
	static bool BuildInputLayout(ID3D11Device* pDevice, ID3DX11EffectTechnique* pTechnique, ID3D11InputLayout** pInputLayout);
	static const std::wstring& GetIlSemanticName(ILSemantic semantic);

private:
	static std::map<ILSemantic, std::wstring> create_map()
	{
		std::map<ILSemantic, std::wstring> m;
		m.insert(std::pair<ILSemantic, std::wstring>(ILSemantic::POSITION, L"POSITION"));
		m.insert(std::pair<ILSemantic, std::wstring>(ILSemantic::NORMAL, L"NORMAL"));
		m.insert(std::pair<ILSemantic, std::wstring>(ILSemantic::COLOR, L"COLOR"));
		m.insert(std::pair<ILSemantic, std::wstring>(ILSemantic::TEXCOORD, L"TEXCOORD"));
		m.insert(std::pair<ILSemantic, std::wstring>(ILSemantic::BINORMAL, L"BINORMAL"));
		m.insert(std::pair<ILSemantic, std::wstring>(ILSemantic::TANGENT, L"TANGENT"));
		m.insert(std::pair<ILSemantic, std::wstring>(ILSemantic::BLENDINDICES, L"BLENDINDICES"));
		m.insert(std::pair<ILSemantic, std::wstring>(ILSemantic::BLENDWEIGHTS, L"BLENDWEIGHTS"));
		return m;
	}
	static std::map<ILSemantic, std::wstring> m_IlSemanticName;
	static const std::wstring m_empty;
};