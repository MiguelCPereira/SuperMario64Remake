#pragma once
//Resharper Disable All

	class PostBlur : public PostProcessingMaterial
	{
	public:
		PostBlur();
		~PostBlur() override = default;
		PostBlur(const PostBlur& other) = delete;
		PostBlur(PostBlur&& other) noexcept = delete;
		PostBlur& operator=(const PostBlur& other) = delete;
		PostBlur& operator=(PostBlur&& other) noexcept = delete;

		void SetIntensity(float intensity);

	protected:
		void Initialize(const GameContext& /*gameContext*/) override {}

	private:
		float m_Intensity{ 0.5f };
	};

