#pragma once
//Resharper Disable All

	class PostGrayscale : public PostProcessingMaterial
	{
	public:
		PostGrayscale();
		~PostGrayscale() override = default;
		PostGrayscale(const PostGrayscale& other) = delete;
		PostGrayscale(PostGrayscale&& other) noexcept = delete;
		PostGrayscale& operator=(const PostGrayscale& other) = delete;
		PostGrayscale& operator=(PostGrayscale&& other) noexcept = delete;
		
	protected:
		void Initialize(const GameContext& /*gameContext*/) override {}
	};

