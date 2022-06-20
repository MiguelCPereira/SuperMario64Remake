#include "stdafx.h"
#include "SpriteFontLoader.h"

SpriteFont* SpriteFontLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	const auto pReader = new BinaryReader();
	pReader->Open(loadInfo.assetFullPath);

	if (!pReader->Exists())
	{
		Logger::LogError(L"Failed to read the assetFile!\nPath: \'{}\'", loadInfo.assetSubPath);
		return nullptr;
	}

	//TODO_W5(L"Implement SpriteFontLoader >> Parse .fnt file");
	//See BMFont Documentation for Binary Layout

	//Parse the Identification bytes (B,M,F)
	pReader->SetBufferPosition(0);
	const auto idByte1 = pReader->Read<byte>();
	const auto idByte2 = pReader->Read<byte>();
	const auto idByte3 = pReader->Read<byte>();
	
	//If Identification bytes doesn't match B|M|F,
	//Log Error (SpriteFontLoader::LoadContent > Not a valid .fnt font) &
	//return nullptr
	if(idByte1 != 'B' || idByte2 != 'M' || idByte3 != 'F')
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > Not a valid .fnt font");
		return nullptr;
	}

	//Parse the version (version 3 required)
	const auto versionByte = pReader->Read<byte>();

	//If version is < 3,
	//Log Error (SpriteFontLoader::LoadContent > Only .fnt version 3 is supported)
	//return nullptr
	if (versionByte < 3)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > Only .fnt version 3 is supported");
		return nullptr;
	}

	//Valid .fnt file >> Start Parsing!
	//use this SpriteFontDesc to store all relevant information (used to initialize a SpriteFont object)
	SpriteFontDesc fontDesc{};


	//**********
	// BLOCK 0 *
	//**********

	//Retrieve the blockId and blockSize
	auto blockId = pReader->Read<byte>();
	auto blockSize = pReader->Read<int>();
	int blockStart = pReader->GetBufferPosition();

	//Retrieve the FontSize [fontDesc.fontSize]
	fontDesc.fontSize = pReader->Read<short>();

	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	//Retrieve the FontName [fontDesc.fontName]
	pReader->MoveBufferPosition(12);
	std::wstringstream ss;
	while(true)
	{
		auto a = pReader->Read<char>();
		if (a == NULL) break;
		ss << a;
	}
	fontDesc.fontName = (std::wstring)ss.str();


	//**********
	// BLOCK 1 *
	//**********

	//Retrieve the blockId and blockSize
	pReader->SetBufferPosition(blockStart + blockSize);
	blockId = pReader->Read<byte>();
	blockSize = pReader->Read<int>();
	blockStart = pReader->GetBufferPosition();

	//Retrieve Texture Width & Height [fontDesc.textureWidth/textureHeight]
	pReader->MoveBufferPosition(4);
	fontDesc.textureWidth = pReader->Read<short>();
	fontDesc.textureHeight = pReader->Read<short>();

	//Retrieve PageCount
	//> if pagecount > 1
	//	> Log Error (Only one texture per font is allowed!)
	//pReader->SetBufferPosition(blockStart + 8);
	const auto pageCount = pReader->Read<short>();
	if(pageCount > 1)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > Only one texture per font is allowed!");
		return nullptr;
	}


	//**********
	// BLOCK 2 *
	//**********

	//Advance to Block2 (Move Reader)
	pReader->SetBufferPosition(blockStart + blockSize);

	//Retrieve the blockId and blockSize
	blockId = pReader->Read<byte>();
	blockSize = pReader->Read<int>();
	blockStart = pReader->GetBufferPosition();

	//Retrieve the PageName (BinaryReader::ReadNullString)
	const auto pageName = pReader->ReadNullString();

	//Construct the full path to the page texture file
	//	>> page texture should be stored next to the .fnt file, pageName contains the name of the texture file
	//	>> full texture path = asset parent_path of .fnt file (see loadInfo.assetFullPath > get parent_path) + pageName (filesystem::path::append)
	//	>> Load the texture (ContentManager::Load<TextureData>) & Store [fontDesc.pTexture]
	auto pageTextFilePath = loadInfo.assetFullPath.parent_path();
	pageTextFilePath.append(pageName);
	fontDesc.pTexture = ContentManager::Load<TextureData>(pageTextFilePath);


	//**********
	// BLOCK 3 *
	//**********

	pReader->SetBufferPosition(blockStart + blockSize);

	//Retrieve the blockId and blockSize
	blockId = pReader->Read<byte>();
	blockSize = pReader->Read<int>();
	blockStart = pReader->GetBufferPosition();

	//Retrieve Character Count (see documentation)
	const auto characterCount = blockSize / 20;

	//Create loop for Character Count, and:
	for (int i = 0; i < characterCount; i++)
	{
		//pReader->SetBufferPosition(blockStart + i * 20);

		//> Retrieve CharacterId (store Local) and cast to a 'wchar_t'
		const auto characterId = pReader->Read<UINT32>();
		const auto castCharId = static_cast<wchar_t>(characterId);

		//> Create instance of FontMetric (struct)
		FontMetric fontMetric{};

		//	> Set Character (CharacterId) [FontMetric::character]
		fontMetric.character = castCharId;

		//	> Retrieve Xposition (store Local)
		const unsigned int posX = pReader->Read<unsigned short>();

		//	> Retrieve Yposition (store Local)
		const unsigned int posY = pReader->Read<unsigned short>();

		//	> Retrieve & Set Width [FontMetric::width]
		fontMetric.width = pReader->Read<unsigned short>();

		//	> Retrieve & Set Height [FontMetric::height]
		fontMetric.height = pReader->Read<unsigned short>();

		//	> Retrieve & Set OffsetX [FontMetric::offsetX]
		fontMetric.offsetX = pReader->Read<short>();

		//	> Retrieve & Set OffsetY [FontMetric::offsetY]
		fontMetric.offsetY = pReader->Read<short>();

		//	> Retrieve & Set AdvanceX [FontMetric::advanceX]
		fontMetric.advanceX = pReader->Read<short>();

		//	> Retrieve & Set Page [FontMetric::page]
		fontMetric.page = pReader->Read<unsigned char>();

		//	> Retrieve Channel (BITFIELD!!!) 
		//		> See documentation for BitField meaning [FontMetric::channel]
		fontMetric.channel = pReader->Read<unsigned char>();

		// Convert to proper data format
		// Channel G & A are already correct, only B & R need to be swapped, but using log2 can make it a bit shorter
		fontMetric.channel = static_cast<unsigned char>(log2(fontMetric.channel));
		if (fontMetric.channel == 0) fontMetric.channel = 2;
		else if (fontMetric.channel == 2) fontMetric.channel = 0;


		//	> Calculate Texture Coordinates using Xposition, Yposition, fontDesc.TextureWidth & fontDesc.TextureHeight [FontMetric::texCoord]
		fontMetric.texCoord = XMFLOAT2(float(posX * fontDesc.textureWidth), float(posY * fontDesc.textureHeight));

		//> Insert new FontMetric to the metrics [font.metrics] map
		//	> key = (wchar_t) charId
		//	> value = new FontMetric
		fontDesc.metrics.insert({ castCharId, fontMetric });

		//(loop restarts till all metrics are parsed)
	}

	//Done!
	delete pReader;
	return new SpriteFont(fontDesc);
}

void SpriteFontLoader::Destroy(SpriteFont* objToDestroy)
{
	SafeDelete(objToDestroy);
}
