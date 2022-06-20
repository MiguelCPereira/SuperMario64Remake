#pragma once

class TextureDataLoader : public ContentLoader<TextureData>
{
public:
	TextureDataLoader() = default;
	~TextureDataLoader() override = default;

	TextureDataLoader(const TextureDataLoader& other) = delete;
	TextureDataLoader(TextureDataLoader&& other) noexcept = delete;
	TextureDataLoader& operator=(const TextureDataLoader& other) = delete;
	TextureDataLoader& operator=(TextureDataLoader&& other) noexcept = delete;

protected:
	TextureData* LoadContent(const ContentLoadInfo& loadInfo) override;
	void Destroy(TextureData* objToDestroy) override;

};

