#include "stdafx.h"
#include "MeshFilterLoader.h"


MeshFilter* MeshFilterLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	const auto pReader = new BinaryReader();
	pReader->Open(loadInfo.assetFullPath);

	if (!pReader->Exists())
		return nullptr;

	//READ OVM FILE
	const int versionMajor = pReader->Read<char>();
	const int versionMinor = pReader->Read<char>();

	MeshFilter* pMeshFilter{ nullptr };

	//OVM 1.1
	if (versionMajor == 1 && versionMinor == 1) pMeshFilter = ParseOVM11(pReader);
	//OVM 2.0
	else if (versionMajor == 2 && versionMinor == 0) pMeshFilter = ParseOVM20(pReader);
	else
	{
		Logger::LogWarning(L"Unsupported OVM Version ({}.{})\n\tFile: \"{}\"", loadInfo.assetFullPath.filename().wstring(), versionMajor, versionMinor);

		delete pReader;
		return nullptr;
	}

	delete pReader;
	return pMeshFilter;
}

void MeshFilterLoader::Destroy(MeshFilter* objToDestroy)
{
	SafeDelete(objToDestroy);
}

#pragma region OVM 1.1 Parser
MeshFilter* MeshFilterLoader::ParseOVM11(BinaryReader* pReader)
{
#pragma region Header Definitions
	//OVM 1.0 Header Definitions
	enum class OVM_HEADER
	{
		END = 0,
		HEADER = 1,
		POSITIONS = 2,
		INDICES = 3,
		NORMALS = 4,
		BINORMALS = 5,
		TANGENTS = 6,
		COLORS = 7,
		TEXCOORDS = 8,
		BLENDINDICES = 9,
		BLENDWEIGHTS = 10,
		ANIMATIONCLIPS = 11,
		SKELETON = 12
	};
#pragma endregion

	//Parsing Logic
	const auto pMeshFilter = new MeshFilter();
	SubMeshFilter subMesh{};
	subMesh.id = 0;

	for (;;)
	{
		const auto blockID = static_cast<OVM_HEADER>(pReader->Read<char>());
		if (blockID == OVM_HEADER::END)
			break;

		const auto blockSize = pReader->Read<unsigned int>();

		switch (blockID)
		{
		case OVM_HEADER::HEADER:
		{
			pMeshFilter->m_MeshName = pReader->ReadString();
			subMesh.name = pMeshFilter->m_MeshName;

			subMesh.vertexCount = pReader->Read<UINT>();
			subMesh.indexCount = pReader->Read<UINT>();
		}
		break;
		case OVM_HEADER::POSITIONS:
		{
			subMesh.layoutElements |= ILSemantic::POSITION;
			subMesh.positions.reserve(subMesh.vertexCount);

			for (UINT i = 0; i < subMesh.vertexCount; ++i)
				subMesh.positions.emplace_back(pReader->Read<XMFLOAT3>());
		}
		break;
		case OVM_HEADER::INDICES:
		{
			subMesh.indices.reserve(subMesh.indexCount);

			for (UINT i = 0; i < subMesh.indexCount; ++i)
				subMesh.indices.emplace_back(pReader->Read<UINT>());
		}
		break;
		case OVM_HEADER::NORMALS:
		{
			subMesh.layoutElements |= ILSemantic::NORMAL;
			subMesh.normals.reserve(subMesh.vertexCount);

			for (UINT i = 0; i < subMesh.vertexCount; ++i)
				subMesh.normals.emplace_back(pReader->Read<XMFLOAT3>());
		}
		break;
		case OVM_HEADER::TANGENTS:
		{
			subMesh.layoutElements |= ILSemantic::TANGENT;
			subMesh.tangents.reserve(subMesh.vertexCount);

			for (UINT i = 0; i < subMesh.vertexCount; ++i)
				subMesh.tangents.emplace_back(pReader->Read<XMFLOAT3>());
		}
		break;
		case OVM_HEADER::BINORMALS:
		{
			subMesh.layoutElements |= ILSemantic::BINORMAL;
			subMesh.binormals.reserve(subMesh.vertexCount);

			for (UINT i = 0; i < subMesh.vertexCount; ++i)
				subMesh.binormals.emplace_back(pReader->Read<XMFLOAT3>());
		}
		break;
		case OVM_HEADER::TEXCOORDS:
		{
			subMesh.layoutElements |= ILSemantic::TEXCOORD;
			subMesh.texCoords.reserve(subMesh.vertexCount);

			for (UINT i = 0; i < subMesh.vertexCount; ++i)
				subMesh.texCoords.emplace_back(pReader->Read<XMFLOAT2>());
		}
		break;
		case OVM_HEADER::COLORS:
		{
			subMesh.layoutElements |= ILSemantic::COLOR;
			subMesh.colors.reserve(subMesh.vertexCount);

			for (UINT i = 0; i < subMesh.vertexCount; ++i)
				subMesh.colors.emplace_back(pReader->Read<XMFLOAT4>());
		}
		break;
		case OVM_HEADER::BLENDINDICES:
		{
			subMesh.layoutElements |= ILSemantic::BLENDINDICES;
			subMesh.blendIndices.reserve(subMesh.vertexCount);

			for (UINT i = 0; i < subMesh.vertexCount; ++i)
				subMesh.blendIndices.emplace_back(pReader->Read<XMFLOAT4>());
		}
		break;
		case OVM_HEADER::BLENDWEIGHTS:
		{
			subMesh.layoutElements |= ILSemantic::BLENDWEIGHTS;
			subMesh.blendWeights.reserve(subMesh.vertexCount);

			for (UINT i = 0; i < subMesh.vertexCount; ++i)
				subMesh.blendWeights.emplace_back(pReader->Read<XMFLOAT4>());
		}
		break;
		case OVM_HEADER::ANIMATIONCLIPS:
		{
			pMeshFilter->m_HasAnimations = true;
			const auto clipCount = pReader->Read<USHORT>();

			for (auto j = 0; j < clipCount; ++j)
			{
				AnimationClip clip{};
				clip.name = pReader->ReadString();
				clip.duration = pReader->Read<float>();
				clip.ticksPerSecond = pReader->Read<float>();

				const auto keyCount = pReader->Read<USHORT>();

				for (auto key = 0; key < keyCount; ++key)
				{
					AnimationKey animKey{};
					animKey.tick = pReader->Read<float>();

					const auto transformCount = pReader->Read<USHORT>();
					for (auto i = 0; i < transformCount; ++i)
					{
						animKey.boneTransforms.emplace_back(pReader->Read<XMFLOAT4X4>());
					}

					clip.keys.emplace_back(animKey);
				}

				pMeshFilter->m_AnimationClips.emplace_back(clip);
			}
		}
		break;
		case OVM_HEADER::SKELETON:
		{
			pMeshFilter->m_BoneCount = pReader->Read<USHORT>();
			pReader->MoveBufferPosition(blockSize - sizeof(USHORT));
		}
		break;
		default:
			pReader->MoveBufferPosition(blockSize);
			break;
		}
	}

	pMeshFilter->m_Meshes.push_back(subMesh);
	return pMeshFilter;
}
#pragma endregion

#pragma region OVM 2.0 Parser
MeshFilter* MeshFilterLoader::ParseOVM20(BinaryReader* pReader)
{
#pragma region Header Definitions
	//OVM 2.0 Header Definitions
	enum class OVM_HEADER
	{
		END = 0,
		HEADER = 1,
		MESHES = 2,
		ANIMATIONS = 3
	};

	enum class OVM_HEADER_MESHES
	{
		END = 0,
		POSITIONS = 1,
		INDICES = 2,
		NORMALS = 3,
		BINORMALS = 4,
		TANGENTS = 5,
		COLORS = 6,
		TEXCOORDS = 7,
		BLENDINDICES = 8,
		BLENDWEIGHTS = 9
	};

	enum class OVM_HEADER_ANIMATIONS
	{
		END = 0,
		CLIPS = 1,
		SKELETON = 2
	};
#pragma endregion

	auto pMeshFilter = new MeshFilter();

	for (;;)
	{
		const auto blockId = static_cast<OVM_HEADER>(pReader->Read<BYTE>());
		if (blockId == OVM_HEADER::END) break;

		/*auto blockSize = */pReader->Read<UINT32>();

		//HEADER BLOCK
		if (blockId == OVM_HEADER::HEADER)
		{
			pMeshFilter->m_MeshName = pReader->ReadString();
			continue; //Advance to Next Block
		}

		//MESHES BLOCK
		if (blockId == OVM_HEADER::MESHES)
		{
			const auto meshCount = pReader->Read<BYTE>();

			//SUB_MESH
			for (auto meshId{ 0 }; meshId < meshCount; ++meshId)
			{
#pragma region SubMeshes
				SubMeshFilter subMesh{};
				/*subMesh.id = */pReader->Read<BYTE>();
				subMesh.id = static_cast<BYTE>(meshId);
				subMesh.name = pReader->ReadString();
				subMesh.vertexCount = pReader->Read<UINT32>();
				subMesh.indexCount = pReader->Read<UINT32>();
				subMesh.uvChannelCount = pReader->Read<UINT32>();

				for (;;)
				{
					const auto meshBlockId = static_cast<OVM_HEADER_MESHES>(pReader->Read<BYTE>());
					if (meshBlockId == OVM_HEADER_MESHES::END)break;

					/*auto meshBlockSize = */pReader->Read<UINT32>();

					switch (meshBlockId)
					{
					case OVM_HEADER_MESHES::POSITIONS:
					{
						subMesh.layoutElements |= ILSemantic::POSITION;
						subMesh.positions.reserve(subMesh.vertexCount);
						for (size_t i{ 0 }; i < subMesh.vertexCount; ++i)
							subMesh.positions.emplace_back(pReader->Read<XMFLOAT3>());
					}
					break;
					case OVM_HEADER_MESHES::INDICES:
					{
						subMesh.indices.reserve(subMesh.indexCount);
						for (size_t i{ 0 }; i < subMesh.indexCount; ++i)
							subMesh.indices.emplace_back(pReader->Read <UINT32>());
					}
					break;
					case OVM_HEADER_MESHES::NORMALS:
					{
						subMesh.layoutElements |= ILSemantic::NORMAL;
						subMesh.normals.reserve(subMesh.vertexCount);
						for (size_t i{ 0 }; i < subMesh.vertexCount; ++i)
							subMesh.normals.emplace_back(pReader->Read<XMFLOAT3>());
					}
					break;
					case OVM_HEADER_MESHES::TANGENTS:
					{
						subMesh.layoutElements |= ILSemantic::TANGENT;
						subMesh.tangents.reserve(subMesh.vertexCount);
						for (size_t i{ 0 }; i < subMesh.vertexCount; ++i)
							subMesh.tangents.emplace_back(pReader->Read<XMFLOAT3>());
					}
					break;
					case OVM_HEADER_MESHES::BINORMALS:
					{
						subMesh.layoutElements |= ILSemantic::BINORMAL;
						subMesh.binormals.reserve(subMesh.vertexCount);
						for (size_t i{ 0 }; i < subMesh.vertexCount; ++i)
							subMesh.binormals.emplace_back(pReader->Read<XMFLOAT3>());
					}
					break;
					case OVM_HEADER_MESHES::COLORS:
					{
						subMesh.layoutElements |= ILSemantic::COLOR;
						subMesh.colors.reserve(subMesh.vertexCount);
						for (size_t i{ 0 }; i < subMesh.vertexCount; ++i)
							subMesh.colors.emplace_back(pReader->Read<XMFLOAT4>());
					}
					break;
					case OVM_HEADER_MESHES::TEXCOORDS:
					{
						subMesh.layoutElements |= ILSemantic::TEXCOORD;
						const auto texCoordCount = static_cast<size_t>(subMesh.vertexCount) * subMesh.uvChannelCount;
						subMesh.texCoords.reserve(texCoordCount);
						for (size_t i{ 0 }; i < texCoordCount; ++i)
							subMesh.texCoords.emplace_back(pReader->Read<XMFLOAT2>());
					}
					break;
					case OVM_HEADER_MESHES::BLENDINDICES:
					{
						subMesh.layoutElements |= ILSemantic::BLENDINDICES;
						subMesh.blendIndices.reserve(subMesh.vertexCount);
						for (size_t i{ 0 }; i < subMesh.vertexCount; ++i)
							subMesh.blendIndices.emplace_back(pReader->Read<XMFLOAT4>());
					}
					break;
					case OVM_HEADER_MESHES::BLENDWEIGHTS:
					{
						subMesh.layoutElements |= ILSemantic::BLENDWEIGHTS;
						subMesh.blendWeights.reserve(subMesh.vertexCount);
						for (size_t i{ 0 }; i < subMesh.vertexCount; ++i)
							subMesh.blendWeights.emplace_back(pReader->Read<XMFLOAT4>());
					}
					break;
					}
				}

				//Add SubMesh to MeshFilter
				pMeshFilter->m_Meshes.emplace_back(subMesh);
#pragma endregion
			}

			continue; //Advance to Next Block
		}

		//ANIMATION BLOCK
		if (blockId == OVM_HEADER::ANIMATIONS)
		{
			for (;;)
			{
				const auto animationBlockId = static_cast<OVM_HEADER_ANIMATIONS>(pReader->Read<BYTE>());
				if (animationBlockId == OVM_HEADER_ANIMATIONS::END) break;

				const auto animationBlockSize = pReader->Read<UINT32>();

#pragma region Animation Blocks
				switch (animationBlockId)
				{
				case OVM_HEADER_ANIMATIONS::CLIPS:
				{
					pMeshFilter->m_HasAnimations = true;
					const auto clipCount = pReader->Read<USHORT>();

					for (auto j = 0; j < clipCount; ++j)
					{
						AnimationClip clip{};
						clip.name = pReader->ReadString();
						clip.duration = pReader->Read<float>();
						clip.ticksPerSecond = pReader->Read<float>();

						const auto keyCount = static_cast<size_t>(pReader->Read<USHORT>());
						clip.keys.reserve(keyCount);

						for (size_t key{ 0 }; key < keyCount; ++key)
						{
							AnimationKey animKey{};
							animKey.tick = pReader->Read<float>();

							const auto transformCount = static_cast<size_t>(pReader->Read<USHORT>());
							animKey.boneTransforms.reserve(transformCount);

							for (size_t i = 0; i < transformCount; ++i)
							{
								animKey.boneTransforms.emplace_back(pReader->Read<XMFLOAT4X4>());
							}

							clip.keys.emplace_back(animKey);
						}

						pMeshFilter->m_AnimationClips.emplace_back(clip);
					}
				}
				break;
				case OVM_HEADER_ANIMATIONS::SKELETON:
				{
					pMeshFilter->m_BoneCount = pReader->Read<UINT16>();
					pReader->MoveBufferPosition(animationBlockSize - sizeof(UINT16));
				}
				break;
				}
#pragma endregion
			}
		}
	}

	return pMeshFilter;
}
#pragma endregion
