#pragma once
class DebugRenderer final
{
public:
	DebugRenderer() = delete;
	~DebugRenderer() = delete;

	DebugRenderer(const DebugRenderer& other) = delete;
	DebugRenderer(DebugRenderer&& other) noexcept = delete;
	DebugRenderer& operator=(const DebugRenderer& other) = delete;
	DebugRenderer& operator=(DebugRenderer&& other) noexcept = delete;


	static void Initialize(const GameContext& gameContext);
	static void ToggleDebugRenderer() { m_RendererEnabled = !m_RendererEnabled; }
	static bool IsEnabled() { return m_RendererEnabled; }
	static void Release();

	//RENDERING
	static void Draw(const SceneContext& sceneContext);
	static void DrawLine(const XMFLOAT3& start, const XMFLOAT3& end, const XMFLOAT4& color = XMFLOAT4{ Colors::Orange });
	static void DrawLine(const XMFLOAT3& start, const XMFLOAT4& colorStart, const XMFLOAT3& end, const XMFLOAT4& colorEnd);
	static void DrawPhysX(PxScene* pScene);
	static void BeginFrame(const SceneSettings& sceneSettings);

private:

	struct DebugRenderGroup
	{
		bool isEnabled{ true };
		bool isStatic{ true };
		UINT bufferSize{};
		std::vector<VertexPosCol> lines{};
		ID3D11Buffer* pVertexBuffer{};

		DebugRenderGroup() = default;
		DebugRenderGroup(bool _isStatic):isStatic(_isStatic){}

		UINT size() const
		{
			return UINT(lines.size());
		}

		void release()
		{
			lines.clear();
			SafeRelease(pVertexBuffer);
		}
	};

	static ID3DX11Effect* m_pEffect;
	static ID3DX11EffectTechnique* m_pTechnique;
	static ID3D11InputLayout *m_pInputLayout;
	static ID3DX11EffectMatrixVariable *m_pWvpVariable;

	static DebugRenderGroup m_PhysXDRG;
	static DebugRenderGroup m_GridDRG;
	static DebugRenderGroup m_UserDRG;

	static bool m_RendererEnabled;
	static GameContext m_GameContext;

	//static void CreateVertexBuffer(ID3D11Device* pDevice);

	static void GenerateGridDRG(UINT numGridLines = 21, float lineSpacing = 1.0f);
	static bool ValidateBufferDRG(DebugRenderGroup& drg);
	static void DrawDRG(const SceneContext& sceneContext, DebugRenderGroup& drg);
	//static void CreateFixedLineList();
};

