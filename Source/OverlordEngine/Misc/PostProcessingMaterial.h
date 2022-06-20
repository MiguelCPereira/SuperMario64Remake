#pragma once
//Resharper Disable All

	class PostProcessingMaterial
	{
	public:
		PostProcessingMaterial(const std::wstring& effectFile);
		virtual ~PostProcessingMaterial();
		PostProcessingMaterial(const PostProcessingMaterial& other) = delete;
		PostProcessingMaterial(PostProcessingMaterial&& other) noexcept = delete;
		PostProcessingMaterial& operator=(const PostProcessingMaterial& other) = delete;
		PostProcessingMaterial& operator=(PostProcessingMaterial&& other) noexcept = delete;

		void InitializeBase(const GameContext& gameContext, UINT materialId);

		//Can be overriden to implement multi-pass effects (like Bloom)
		virtual void Draw(const SceneContext& sceneContext, RenderTarget* pSource);

		RenderTarget* GetOutput() const { return m_pOutputTarget; }
		UINT GetMaterialId() const { return m_MaterialId; }
		void ResetMaterialId() { m_MaterialId = UINT_MAX; }
		void SetIsEnabled(bool isEnabled) { m_IsEnabled = isEnabled; }
		bool IsEnabled() const { return m_IsEnabled; }

	protected:
		virtual void Initialize(const GameContext& gameContext) = 0;
		virtual void UpdateBaseEffectVariables(const SceneContext& sceneContext, RenderTarget* pSource);

		//Invokes FullScreen Quad DrawCall to Destination RenderTarget (using the given Shader Technique)
		void DrawPass(const SceneContext& sceneContext, ID3DX11EffectTechnique* pTechnique, RenderTarget* pDestination);

		GameContext m_GameContext;
		RenderTarget* m_pOutputTarget{};

		ID3DX11Effect* m_pBaseEffect{};
		ID3DX11EffectTechnique* m_pBaseTechnique{};

	private:
		std::wstring m_EffectFile{};
		bool m_IsEnabled{ true };
		UINT m_MaterialId{UINT_MAX};

		static constexpr UINT m_VertexCount{ 4 };
		static ID3D11Buffer* m_pDefaultVertexBuffer;
		static ID3D11InputLayout* m_pDefaultInputLayout;
		static UINT m_References;
	};

