#include "stdafx.h"
#include "DebugRenderer.h"

ID3DX11Effect* DebugRenderer::m_pEffect = nullptr;
ID3DX11EffectTechnique* DebugRenderer::m_pTechnique = nullptr;
ID3D11InputLayout* DebugRenderer::m_pInputLayout = nullptr;
ID3DX11EffectMatrixVariable* DebugRenderer::m_pWvpVariable = nullptr;

bool DebugRenderer::m_RendererEnabled = true;
GameContext DebugRenderer::m_GameContext = {};

DebugRenderer::DebugRenderGroup DebugRenderer::m_PhysXDRG = {false};
DebugRenderer::DebugRenderGroup DebugRenderer::m_GridDRG = {true};
DebugRenderer::DebugRenderGroup DebugRenderer::m_UserDRG = {false};

void DebugRenderer::Release()
{
	SafeRelease(m_pInputLayout);

	m_PhysXDRG.release();
	m_GridDRG.release();
	m_UserDRG.release();
}

void DebugRenderer::Initialize(const GameContext& gameContext)
{

//#ifdef _DEBUG
//	m_RendererEnabled = true;
//#else
//	m_RendererEnabled = false;
//#endif

	m_GameContext = gameContext;

	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"Effects\\DebugRenderer.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
	EffectHelper::BuildInputLayout(m_GameContext.d3dContext.pDevice, m_pTechnique, &m_pInputLayout);

	m_pWvpVariable = m_pEffect->GetVariableBySemantic("WORLDVIEWPROJECTION")->AsMatrix();
	if (!m_pWvpVariable->IsValid())
		Logger::LogWarning(L"Debug Renderer: Invalid Shader Variable! (WVP)");

	GenerateGridDRG();
}

void DebugRenderer::GenerateGridDRG(UINT numGridLines, float lineSpacing)
{
	m_GridDRG.lines.clear();

	auto lineColor{ XMFLOAT4{Colors::LightGray} };
	const auto lineLength{ (numGridLines - 1) * lineSpacing };
	const float startOffset{ -((int(numGridLines) / 2) * lineSpacing) };

	m_GridDRG.lines.reserve(size_t(numGridLines) * 2 + 6);

	//*AXIS
	m_GridDRG.lines.emplace_back(XMFLOAT3(0, 0, 0), XMFLOAT4{ Colors::DarkRed });
	m_GridDRG.lines.emplace_back(XMFLOAT3(30, 0, 0), XMFLOAT4{ Colors::DarkRed });
	m_GridDRG.lines.emplace_back(XMFLOAT3(0, 0, 0), XMFLOAT4{ Colors::DarkGreen });
	m_GridDRG.lines.emplace_back(XMFLOAT3(0, 30, 0), XMFLOAT4{ Colors::DarkGreen });
	m_GridDRG.lines.emplace_back(XMFLOAT3(0, 0, 0), XMFLOAT4{ Colors::DarkBlue });
	m_GridDRG.lines.emplace_back(XMFLOAT3(0, 0, 30), XMFLOAT4{ Colors::DarkBlue });

	//*GRID
	for (unsigned int i = 0; i < numGridLines; ++i)
	{
		//VERTICAL
		const float lineOffset = startOffset + lineSpacing * i;
		auto vertStart = XMFLOAT3(startOffset, 0, lineOffset);
		m_GridDRG.lines.emplace_back(vertStart, lineColor);
		vertStart.x += lineLength;
		m_GridDRG.lines.emplace_back(vertStart, lineColor);

		//HORIZONTAL
		vertStart = XMFLOAT3(lineOffset, 0, startOffset);
		m_GridDRG.lines.emplace_back(vertStart, lineColor);
		vertStart.z += lineLength;
		m_GridDRG.lines.emplace_back(vertStart, lineColor);
	}
}

bool DebugRenderer::ValidateBufferDRG(DebugRenderGroup& drg)
{
	const auto drgSize = drg.size();

	if (!drg.isStatic && drgSize <= 0)
		return false;

	if (drg.isStatic && drg.pVertexBuffer != nullptr)
		return true;

	if (drg.pVertexBuffer == nullptr || drg.bufferSize< drgSize)
	{
		SafeRelease(drg.pVertexBuffer);

		drg.bufferSize = drgSize;

		//Vertexbuffer
		D3D11_BUFFER_DESC buffDesc{};
		buffDesc.Usage = drg.isStatic ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
		buffDesc.ByteWidth = sizeof(VertexPosCol) * drgSize;
		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffDesc.CPUAccessFlags = drg.isStatic ? 0 : D3D11_CPU_ACCESS_WRITE;
		buffDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = drg.lines.data();

		HANDLE_ERROR(m_GameContext.d3dContext.pDevice->CreateBuffer(&buffDesc, &initData, &drg.pVertexBuffer))

		return true;
	}

	//Update Dynamic
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	const auto pDeviceContext = m_GameContext.d3dContext.pDeviceContext;

	pDeviceContext->Map(drg.pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
	memcpy(&static_cast<VertexPosCol*>(mappedResource.pData)[0], drg.lines.data(), sizeof(VertexPosCol) * drgSize);
	pDeviceContext->Unmap(drg.pVertexBuffer, 0);

	return true;
}

void DebugRenderer::DrawDRG(const SceneContext& sceneContext, DebugRenderGroup& drg)
{
	if (!drg.isEnabled) return;
	ValidateBufferDRG(drg);

	const auto pDeviceContext = m_GameContext.d3dContext.pDeviceContext;

	//Set Render Pipeline
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	constexpr UINT stride = sizeof(VertexPosCol);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &drg.pVertexBuffer, &stride, &offset);
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	auto& viewProj = sceneContext.pCamera->GetViewProjection();
	m_pWvpVariable->SetMatrix(&viewProj._11);

	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pTechnique->GetDesc(&techDesc);
	for (unsigned int i = 0; i < techDesc.Passes; ++i)
	{
		m_pTechnique->GetPassByIndex(i)->Apply(0, pDeviceContext);
		pDeviceContext->Draw(drg.size(), 0);
	}

	//Clear Dynamic RenderGroup lines
	if (!drg.isStatic)
		drg.lines.clear();
}

void DebugRenderer::DrawLine(const XMFLOAT3& start, const XMFLOAT3& end, const XMFLOAT4& color)
{
	if (!m_UserDRG.isEnabled)
		return;

	DrawLine(start, color, end, color);
}

void DebugRenderer::DrawLine(const XMFLOAT3& start, const XMFLOAT4& colorStart, const XMFLOAT3& end, const XMFLOAT4& colorEnd)
{
	if (!m_UserDRG.isEnabled)
		return;

	m_UserDRG.lines.emplace_back(VertexPosCol(start, colorStart));
	m_UserDRG.lines.emplace_back(VertexPosCol(end, colorEnd));
}

void DebugRenderer::DrawPhysX(PxScene* pScene)
{
	if (!m_PhysXDRG.isEnabled)
		return;

	//m_pPhysxDebugScene = pScene;
	const auto pxDebugRenderer = &pScene->getRenderBuffer();
	const auto pxDebugLines = pxDebugRenderer->getLines();
	const auto pxLineCount = pxDebugRenderer->getNbLines();

	m_PhysXDRG.lines.reserve(size_t(pxLineCount) * 2);

	for (unsigned int i = 0; i < pxLineCount; ++i)
	{
		const auto& line = pxDebugLines[i];
		m_PhysXDRG.lines.emplace_back(PhysxHelper::ToXMFLOAT3(line.pos0), PhysxHelper::ColorToXMFLOAT4(line.color0));
		m_PhysXDRG.lines.emplace_back(PhysxHelper::ToXMFLOAT3(line.pos1), PhysxHelper::ColorToXMFLOAT4(line.color1));
	}
}

void DebugRenderer::BeginFrame(const SceneSettings& sceneSettings)
{
	//Update Draw Flags
	m_GridDRG.isEnabled = m_RendererEnabled && sceneSettings.drawGrid;
	m_PhysXDRG.isEnabled = m_RendererEnabled && sceneSettings.drawPhysXDebug;
	m_UserDRG.isEnabled = m_RendererEnabled && sceneSettings.drawUserDebug;
}

void DebugRenderer::Draw(const SceneContext& sceneContext)
{
	if (!m_RendererEnabled)return;

	DrawDRG(sceneContext, m_GridDRG);
	DrawDRG(sceneContext, m_PhysXDRG);
	DrawDRG(sceneContext, m_UserDRG);
}
