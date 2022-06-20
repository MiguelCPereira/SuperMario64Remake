#pragma once
//Resharper Disable All

struct VertexPosCol
{
public:

	VertexPosCol() = default;
	VertexPosCol(XMFLOAT3 pos, XMFLOAT4 col) :
		Position(pos), Color(col) {}

	XMFLOAT3 Position = {};
	XMFLOAT4 Color = {};
};

struct VertexPosNormCol
{
public:

	VertexPosNormCol() = default;
	VertexPosNormCol(XMFLOAT3 pos, XMFLOAT3 norm, XMFLOAT4 col) :
		Position(pos), Normal(norm), Color(col) {}

	XMFLOAT3 Position = {};
	XMFLOAT3 Normal = {};
	XMFLOAT4 Color = {};
};

struct VertexPosNormTex
{
public:

	VertexPosNormTex() = default;
	VertexPosNormTex(XMFLOAT3 pos, XMFLOAT3 norm, XMFLOAT2 texCoord) :
		Position(pos), Normal(norm), TexCoord(texCoord) {}

	XMFLOAT3 Position = {};
	XMFLOAT3 Normal = {};
	XMFLOAT2 TexCoord = {};
};

struct QuadPosNormCol
{
public:

	QuadPosNormCol() = default;
	QuadPosNormCol(VertexPosNormCol vert1, VertexPosNormCol vert2, VertexPosNormCol vert3, VertexPosNormCol vert4) :
		Vertex1(vert1), Vertex2(vert2), Vertex3(vert3), Vertex4(vert4) {}

	VertexPosNormCol Vertex1 = {};
	VertexPosNormCol Vertex2 = {};
	VertexPosNormCol Vertex3 = {};
	VertexPosNormCol Vertex4 = {};
};

struct TrianglePosNormCol
{
public:

	TrianglePosNormCol() = default;
	TrianglePosNormCol(VertexPosNormCol vert1, VertexPosNormCol vert2, VertexPosNormCol vert3) :
		Vertex1(vert1), Vertex2(vert2), Vertex3(vert3) {}

	VertexPosNormCol Vertex1 = {};
	VertexPosNormCol Vertex2 = {};
	VertexPosNormCol Vertex3 = {};
};

//Post Processing
struct VertexPosTex
{
public:

	VertexPosTex() = default;
	VertexPosTex(XMFLOAT3 pos, XMFLOAT2 uv) :
		Position(pos), UV(uv) {}

	XMFLOAT3 Position = {};
	XMFLOAT2 UV = {};
};

struct QuadPosTex
{
public:

	QuadPosTex() = default;
	QuadPosTex(VertexPosTex vert1, VertexPosTex vert2, VertexPosTex vert3, VertexPosTex vert4) :
		Vertex1(vert1), Vertex2(vert2), Vertex3(vert3), Vertex4(vert4) {}

	VertexPosTex Vertex1 = {};
	VertexPosTex Vertex2 = {};
	VertexPosTex Vertex3 = {};
	VertexPosTex Vertex4 = {};
};

//Sprite Rendering
struct VertexSprite
{
	UINT TextureId;
	XMFLOAT4 TransformData;
	XMFLOAT4 TransformData2;
	XMFLOAT4 Color;

	bool Equals(const VertexSprite& source) const
	{
		if (source.TextureId != TextureId)return false;
		if (!MathHelper::XMFloat4Equals(source.TransformData, TransformData))return false;
		if (!MathHelper::XMFloat4Equals(source.TransformData2, TransformData2))return false;
		if (!MathHelper::XMFloat4Equals(source.Color, Color))return false;

		return true;
	}
};

//Text Rendering
struct VertexText
{
	UINT channelId{};
	XMFLOAT3 position{};
	XMFLOAT4 color{};
	XMFLOAT2 texCoord{};
	XMFLOAT2 characterDimension{};
};

//Particle Rendering
struct VertexParticle
{
	XMFLOAT3 Position{};
	XMFLOAT4 Color{XMFLOAT4{Colors::White}};
	float Size{5.f};
	float Rotation{0.f};
};