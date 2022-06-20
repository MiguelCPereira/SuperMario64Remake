#pragma once

class SpriteFont;

class SpriteFontLoader : public ContentLoader<SpriteFont>
{
public:
	SpriteFontLoader() = default;
	~SpriteFontLoader() override = default;
	SpriteFontLoader(const SpriteFontLoader& other) = delete;
	SpriteFontLoader(SpriteFontLoader&& other) noexcept = delete;
	SpriteFontLoader& operator=(const SpriteFontLoader& other) = delete;
	SpriteFontLoader& operator=(SpriteFontLoader&& other) noexcept = delete;

protected:
	SpriteFont* LoadContent(const ContentLoadInfo& loadInfo) override;
	void Destroy(SpriteFont* objToDestroy) override;
};

