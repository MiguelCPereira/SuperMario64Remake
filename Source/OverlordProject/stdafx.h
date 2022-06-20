#pragma once
#include <vld.h>

#pragma region WINDOWS
//PhysX:you must suppress its definition of the macros min and max, as these are common C++ method names in the SDK
#define NOMINMAX
// Windows Header Files:
//Core
#include <codecvt>
#include <cwctype>
#include <clocale>
#include <memory>
#include <string>
#include "wchar.h"
#include <Windows.h>

//Streams
#include <fstream>
#include <iostream>
#include <sstream>

//Containers
#include <map>
#include <unordered_map>
#include <vector>

//Misc
#include <filesystem>
#include <format>
#include <rpc.h>
#include <source_location>
#include <typeinfo>
#include <codeanalysis\warnings.h>
namespace fs = std::filesystem;
#pragma endregion

#pragma region DIRECTX
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

//DirectXMath
#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
using namespace DirectX;
#pragma endregion

#pragma region DIRECTX Extensions
//**FX11 (Helper for loading Effects (D3DX11))
//https://github.com/microsoft/FX11
#include <d3dx11effect.h>

//*DirectXTex (Helper for loading Textures (D3DX11))
//https://github.com/microsoft/DirectXTex
#pragma warning(push)
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS)
#include <DirectXTex.h>
#pragma warning(pop)

//*XINPUT 9.1
#include <Xinput.h>
#pragma endregion

#pragma region PHYSX
//NVIDIA PhysX
//https://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Index.html
#pragma warning(push)
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include <PxPhysicsAPI.h>
#pragma warning(pop)
using namespace physx;
#pragma endregion

#pragma region FMOD
//FMOD (v1.10)
//https://www.fmod.com/
#pragma warning(push)
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#pragma warning(disable: 4505 26812)
#include "fmod.hpp" //Precompiled Header
#include "fmod_errors.h"
#pragma warning(pop)
#pragma endregion

#pragma region IMGUI
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#pragma endregion

//Engine Includes
#include "OverlordAPI.h"