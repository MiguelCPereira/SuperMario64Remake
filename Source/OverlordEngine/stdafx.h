#pragma once

#pragma region Third-Party Includes/Libs
#include <vld.h>

#pragma region WINDOWS
//PhysX:you must suppress its definition of the macros min and max, as these are common C++ method names in the SDK
#define NOMINMAX
// Windows Header Files:
//Core
#include <windows.h>
#include <wchar.h>
#include <string>
#include <memory>
#include <locale.h>
#include <codecvt>
#include <cwctype>

//Streams
#include <iostream>
#include <fstream>
#include <sstream>

//Containers
#include <vector>
#include <map>
#include <unordered_map>

//Misc
#include <source_location>
#include <filesystem>
#include <format>
#include <codeanalysis\warnings.h>
#include <rpc.h>
#include <typeinfo>
namespace fs = std::filesystem;
#pragma endregion

#pragma region DIRECTX
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

//DirectXMath
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
using namespace DirectX;
#pragma endregion

#pragma region DIRECTX Extensions
//**FX11 (Helper for loading Effects (D3DX11))
//https://github.com/microsoft/FX11
#include <d3dx11effect.h>
#pragma comment(lib, "Effects11.lib")


//*DirectXTex (Helper for loading Textures (D3DX11))
//https://github.com/microsoft/DirectXTex
#pragma warning(push)
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS)
#include <DirectXTex.h>
#pragma warning(pop)
#pragma comment(lib, "DirectXTex.lib")


//*XINPUT 9.1
#include <Xinput.h>
#pragma comment(lib, "XINPUT9_1_0.LIB")
#pragma endregion

#pragma region PHYSX
//NVIDIA PhysX
//https://gameworksdocs.nvidia.com/PhysX/4.0/documentation/PhysXGuide/Index.html
#pragma warning(push)
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include <PxPhysicsAPI.h>
#pragma warning(pop)
using namespace physx;

#pragma comment(lib, "PhysX_64.lib")
#pragma comment(lib, "PhysXFoundation_64.lib")
#pragma comment(lib, "PhysXCommon_64.lib")
#pragma comment(lib, "PhysXExtensions_static_64.lib")
#pragma comment(lib, "PhysXPvdSDK_static_64.lib")
#pragma comment(lib, "PhysXCharacterKinematic_static_64.lib")

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

#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "fmodL64_vc.lib")
#else
#pragma comment(lib, "fmod64_vc.lib")
#endif
#pragma endregion

#pragma region IMGUI
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#pragma comment(lib, "ImGui_x64.lib")
#pragma endregion

#pragma endregion
#include "Lab_Todos.h"
#include "OverlordAPI.h"
