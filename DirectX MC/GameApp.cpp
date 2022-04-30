#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include"GameLoader.h"

using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance),
	m_pBasicEffect(std::make_unique<BasicEffect>()),
	m_pSkyEffect(std::make_unique<SkyEffect>()),
	m_pShadowEffect(std::make_unique<ShadowEffect>()),
	m_pDebugEffect(std::make_unique<DebugEffect>()),
	m_EnableNormalMap(true),
	m_EnableDebug(true),
	m_GrayMode(true),
	m_EnableClip(true),
	m_SlopeIndex()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// 务必先初始化所有渲染状态，以供下面的特效使用
	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!m_pBasicEffect->InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_pSkyEffect->InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_pShadowEffect->InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_pDebugEffect->InitAll(m_pd3dDevice.Get()))
		return false;

	if (!InitResource())
		return false;

#ifndef USE_IMGUI
	// 初始化鼠标，键盘不需要
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
#endif

	return true;
}

void GameApp::OnResize()
{
	// 释放D2D的相关资源
	m_pColorBrush.Reset();
	m_pd2dRenderTarget.Reset();

	D3DApp::OnResize();

	// 摄像机变更显示
	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_pBasicEffect->SetProjMatrix(m_pCamera->GetProjXM());
	}


}

float trans(float k)
{
	float y;
	//while (k < -0.25f)k += 1;
	//while (k > 0.75f)k -= 1;
	while (k < -0)k += 1;
	while (k > 2.0f)k -= 1;
	if (k < 0.5f)
	{
		y = k;
	}
	else
	{
		y = (-k + 1);
	}
	return y;
};

void GameApp::UpdateScene(float dt)
{
	static const DirectX::XMFLOAT3 lightDirs[] = {
		XMFLOAT3(1.0f / sqrtf(2.0f), -1.0f / sqrtf(2.0f), 0.0f),
		XMFLOAT3(3.0f / sqrtf(13.0f), -2.0f / sqrtf(13.0f), 0.0f),
		XMFLOAT3(2.0f / sqrtf(5.0f), -1.0f / sqrtf(5.0f), 0.0f),
		XMFLOAT3(3.0f / sqrtf(10.0f), -1.0f / sqrtf(10.0f), 0.0f),
		XMFLOAT3(4.0f / sqrtf(17.0f), -1.0f / sqrtf(17.0f), 0.0f)
	};

	static int creatureNum = 20;
	//生物随机生成
	{
		if (m_pAnimals.size() < creatureNum)
		{
			m_pAnimals.push_back(std::shared_ptr<Animal>(new Animal));
			m_pAnimals.back()->init_Animal(m_pd3dDevice.Get(), m_pMainWorld.get());
			auto randPos = m_pMainWorld->GetHighPoint(rand() % 100, rand() % 100);
			m_pAnimals.back()->GetPosition() = XMFLOAT3{
				static_cast<float>(randPos.x),
				static_cast<float>(randPos.y+2),
				static_cast<float>(randPos.z) };
		}
		else if (m_pAnimals.size() > creatureNum)
		{
			m_pAnimals.back()->shouldRemove = true;
		}
	}
	// 更新摄像机
	UpdateCamera();

	m_Controller.Update(dt);
	for (auto iter = m_pAnimals.begin(); iter != m_pAnimals.end();)
	{
		iter->get()->Process(dt);
		if (iter->get()->shouldRemove)
		{
			iter = m_pAnimals.erase(iter);
		}
		else
		{
			++iter;
		}
	}
	//
	// 更新物体与视锥体碰撞
	//
	//BoundingFrustum frustum;
	//BoundingFrustum::CreateFromMatrix(frustum, m_pCamera->GetProjMatrixXM());
	//frustum.Transform(frustum, m_pCamera->GetLocalToWorldMatrixXM());
	//m_SceneModel.FrustumCulling(frustum);

	static bool enable_lighttime = false;
	static float theta = 0;
	static float phi = 0;
	if (ImGui::Begin("Setting"))
	{
		ImGui::Checkbox("Enable Debug", &m_EnableDebug);
		ImGui::Checkbox("Enable Clip", &m_EnableClip);
		ImGui::SliderInt("Creature number", &creatureNum, 1, 100);
		static float r0 = 40;
		if (ImGui::SliderFloat("R0", &r0, 0, 100))
		{
			m_pBasicEffect->SetET_r0(r0);
			m_pShadowEffect->SetET_r0(r0);
		}

		ImGui::SliderInt("Render Distance", &BlockRender::GetInstance()->m_RenderRange, 1, 100);
		static XMFLOAT3 sunLightColor = { 1.0f,1.0f,1.0f };
		static float sunRadius = 20;
		if (ImGui::SliderFloat("Sun Radius", &sunRadius, 1, 100) ||
			ImGui::ColorEdit3("SunLight", &sunLightColor.x))
		{
			m_DirLights[0].diffuse = XMFLOAT4(sunLightColor.x, sunLightColor.y, sunLightColor.z, 1.0f);
			m_pSkyBox->SetSunInfo(*m_pSkyEffect, sunRadius, sunLightColor);
		}
		ImGui::SliderFloat("Light Pos theta", &theta, 0, XM_2PI);
		ImGui::SliderFloat("Light Pos phi", &phi, 0, XM_2PI);
		ImGui::Checkbox("Run Time", &enable_lighttime);

		ImGui::Text("Position %.1f   %.1f   %.1f",
			m_pPlayer->GetPosition().x,
			m_pPlayer->GetPosition().y,
			m_pPlayer->GetPosition().z);

		if (ImGui::Button("SaveGame"))
		{
			GameLoader::GetInstance()->Clear();
			auto& desc = GameLoader::GetInstance()->desc;
			m_pMainWorld->Save(&desc);
			m_pPlayer->Save(&desc.playerData);
			for (auto& animal : m_pAnimals)
			{
				WorldDesc::CreatureDesc a_desc;
				animal->Save(&a_desc);
				desc.creatureData.push_back(a_desc);
			}
			GameLoader::GetInstance()->SaveGame("World1");
		}
		if (ImGui::Button("Regenerate World"))
		{
			m_pMainWorld->GenerateWorld();
		}
	}
	ImGui::End();

	m_pBasicEffect->SetViewMatrix(m_pCamera->GetViewXM());
	m_pBasicEffect->SetEyePos(m_pCamera->GetPosition());
	m_pShadowEffect->SetEyePos(m_pCamera->GetPosition());

	// 更新光照
	if (enable_lighttime)
	{
		theta += dt * XM_2PI / 40.0f;
		if (theta > XM_2PI)
		{
			theta -= XM_2PI;
		}
	}
	XMFLOAT2 sunPos{ theta,phi };//球面坐标 theta phi
	XMFLOAT2 playerPos{};
	auto worldsize = m_pMainWorld->GetChunk()->ClunkBox();
	auto realpos = m_pPlayer->CreaturePos;


	playerPos.x = XM_2PI * realpos.z / worldsize.z;
	playerPos.y = XM_2PI * realpos.x / worldsize.x;

	XMFLOAT2 relevantPos;
	XMStoreFloat2(&relevantPos, XMLoadFloat2(&sunPos) - XMLoadFloat2(&playerPos));

	float k1 = cos(relevantPos.x);
	float k2 = cos(relevantPos.y);
	//if (k1 < 0 && k2 < 0)
	//{
	//	k2 = -k2;
	//}

	XMFLOAT3 sunDir(
		std::sin(relevantPos.y),
		(k1 + k2 + k1 * k2 - 1) / 2,
		std::sin(relevantPos.x));


	{
		XMVECTOR dirVec = XMVector3Normalize(XMLoadFloat3(&sunDir));
		XMStoreFloat3(&m_DirLights[0].direction, -dirVec);
		m_pBasicEffect->SetDirLight(0, m_DirLights[0]);

		XMStoreFloat3(&sunDir, dirVec);

		float s1 = atan2f(sunDir.x, sunDir.z);
		float s2 = acos(XMVector3Dot(g_XMIdentityR1, dirVec).m128_f32[0]);

		//m_pSun->GetTransform().SetPosition(sunDir);
		XMMATRIX r0;
		if (sunDir.y > 0)
			r0 = XMMatrixRotationY(-s1);
		else
			r0 = XMMatrixRotationY(s1);

		XMMATRIX r1 = XMMatrixRotationX(s2);
		XMMATRIX r2 = XMMatrixRotationY(s1);

		static bool lastPositive = true;
		static XMMATRIX rf = XMMatrixRotationY(0);

		XMMATRIX skyRoate;

		if (sunDir.y < 0 && lastPositive == true)
		{
			lastPositive = false;
			XMMATRIX origin = rf * XMMatrixRotationY(-s1);
			rf = origin * XMMatrixInverse(nullptr, r0);

		}
		else if (sunDir.y > 0 && lastPositive == false)
		{
			lastPositive = true;
			XMMATRIX origin = rf * XMMatrixRotationY(s1);
			rf = XMMatrixInverse(nullptr, r0) * origin;

		}
		skyRoate = rf * r0 * r1 * r2;

		XMStoreFloat3(&sunDir, dirVec * 200);
		XMMATRIX SunView = XMMatrixTranslation(sunDir.x, sunDir.y, sunDir.z);
		SunView *= 40;
		m_pSkyBox->SetSkyRoate(skyRoate);
		m_pSkyBox->SetSunView(SunView);

		XMStoreFloat3((XMFLOAT3*)&m_DirLights[0].ambient, XMVECTOR{ 1.0f,1.0f,1.0f, 0.0f }*(cos(s2) + 1.5));

	}

	//
	// 投影区域为正方体，以原点为中心，以方向光为+Z朝向
	//
	XMVECTOR dirVec = XMLoadFloat3(&m_DirLights[0].direction);
	dirVec *= 20.0f * (-2.0f);
	dirVec += m_pCamera->GetPositionXM();
	XMMATRIX LightView = XMMatrixLookAtLH(dirVec, m_pCamera->GetPositionXM(), g_XMIdentityR1);
	m_pShadowEffect->SetViewMatrix(LightView);
	// 将NDC空间 [-1, +1]^2 变换到纹理坐标空间 [0, 1]^2
	static XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	// S = V * P * T
	m_pBasicEffect->SetShadowTransformMatrix(LightView * XMMatrixOrthographicLH(40.0f, 40.0f, 20.0f, 60.0f) * T);

	// 退出程序，这里应向窗口发送销毁信息
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Silver));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ******************
	// 绘制到阴影贴图
	//
	m_pShadowMap->Begin(m_pd3dImmediateContext.Get(), nullptr);
	{
		DrawScene(m_pShadowEffect.get());
	}
	m_pShadowMap->End(m_pd3dImmediateContext.Get());

	// ******************
	// 正常绘制场景
	//
	m_pBasicEffect->SetTextureShadowMap(m_pShadowMap->GetOutputTexture());
	DrawScene(m_pBasicEffect.get(), m_EnableNormalMap);

	// 绘制天空盒
	m_pSkyEffect->SetRenderDefault(m_pd3dImmediateContext.Get());
	m_pSkyBox->Draw(m_pd3dImmediateContext.Get(), *m_pSkyEffect, *m_pCamera);

	// 解除深度缓冲区绑定
	m_pBasicEffect->SetTextureShadowMap(nullptr);
	m_pBasicEffect->Apply(m_pd3dImmediateContext.Get());

	if (m_EnableDebug)
	{
		if (ImGui::Begin("Depth Buffer", &m_EnableDebug))
		{
			m_pDebugEffect->SetRenderOneComponentGray(m_pd3dImmediateContext.Get(), 0);
			m_pGrayShadowMap->Begin(m_pd3dImmediateContext.Get(), Colors::Black);
			{
				m_FullScreenDebugQuad.Draw(m_pd3dImmediateContext.Get(), m_pDebugEffect.get());
			}
			m_pGrayShadowMap->End(m_pd3dImmediateContext.Get());
			// 解除绑定
			m_pDebugEffect->SetTextureDiffuse(nullptr);
			m_pDebugEffect->Apply(m_pd3dImmediateContext.Get());
			ImVec2 winSize = ImGui::GetWindowSize();
			float smaller = (std::min)((winSize.x - 20) / AspectRatio(), winSize.y - 36);
			ImGui::Image(m_pGrayShadowMap->GetOutputTexture(), ImVec2(smaller * AspectRatio(), smaller));
		}
		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HR(m_pSwapChain->Present(0, 0));
}

void GameApp::DrawScene(BasicEffect* pBasicEffect, bool enableNormalMap)
{

	pBasicEffect->SetRenderWithNormalMap(m_pd3dImmediateContext.Get(), IEffect::RenderInstance);
	m_pMainWorld->Draw(m_pd3dImmediateContext.Get(), pBasicEffect, m_EnableClip);


	pBasicEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), IEffect::RenderObject);
	m_pPlayer->Draw(m_pd3dImmediateContext.Get(), pBasicEffect);
	//m_pSun->Draw(m_pd3dImmediateContext.Get(), pBasicEffect);
	for (auto& animal : m_pAnimals)
	{
		if (animal->shouldRender)
			animal->Draw(m_pd3dImmediateContext.Get(), pBasicEffect);
	}
}

void GameApp::DrawScene(ShadowEffect* pShadowEffect)
{
	//世界
	pShadowEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), IEffect::RenderInstance);
	m_pMainWorld->Draw(m_pd3dImmediateContext.Get(), pShadowEffect, false);

	if (m_CameraMode == CameraMode::FirstPerson)m_pPlayer->ThirdPersonModel();
	pShadowEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), IEffect::RenderObject);
	m_pPlayer->Draw(m_pd3dImmediateContext.Get(), pShadowEffect);
	if (m_CameraMode == CameraMode::FirstPerson)m_pPlayer->FirstPersonModel();

	pShadowEffect->SetRenderDefault(m_pd3dImmediateContext.Get(), IEffect::RenderObject);
	for (auto& animal : m_pAnimals)
	{
		if (animal->shouldRender)
		{
			animal->HpModel(false);
			animal->Draw(m_pd3dImmediateContext.Get(), pShadowEffect);
			animal->HpModel(true);
		}
	}
}

bool GameApp::InitResource()
{

	//初始化世界
	auto world = std::shared_ptr<World>(new World);


	m_pMainWorld = world;
	m_pBasicEffect->SetET_r0(50);
	m_pShadowEffect->SetET_r0(50);
	// ******************
	// 生物
	//
	auto player = std::shared_ptr<Player>(new Player);
	m_pPlayer = player;
	player->init_Player(m_pd3dDevice.Get(), world.get());


	m_pAnimals.resize(1);
	m_pAnimals[0] = std::shared_ptr<Animal>(new Animal);
	m_pAnimals[0]->init_Animal(m_pd3dDevice.Get(), world.get());

	if (GameLoader::GetInstance()->LoadGame("World1"))
	{
		world->Load(GameLoader::GetInstance()->GetDesc());
		player->Load(GameLoader::GetInstance()->GetDesc()->playerData);
		for (auto& creatures : GameLoader::GetInstance()->GetDesc()->creatureData)
		{
			m_pAnimals.push_back(std::shared_ptr<Animal>(new Animal));
			m_pAnimals.back()->Load(creatures, world.get());
		}
	}
	else
	{
		world->GenerateWorld();
	}
	world->init(m_pd3dDevice.Get());


	auto camera = std::shared_ptr<FirstPersonCamera>(new FirstPersonCamera);
	m_pCamera = camera;

	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetFrustum(XM_PI / 2.5, AspectRatio(), 0.1f, 1000.0f); 
	camera->LookTo(player->GetPosition(), XMFLOAT3{ 0.5,0,0.5 }, XMFLOAT3{ 0,1,0 });

	m_Controller.init(player.get(), m_pd3dDevice.Get(), &m_pCamera, world.get());

	////m_pSun = std::shared_ptr<GameObject>(new GameObject);

	//Model sphereModel;
	//ComPtr<ID3D11ShaderResourceView> SunTex;
	//HR(CreateWICTextureFromFile(m_pd3dDevice.Get(), L"..\\Texture\\Sun.png", nullptr, SunTex.GetAddressOf()));
	//sphereModel.SetMesh(m_pd3dDevice.Get(), Geometry::CreateSphere(50.0f));
	//sphereModel.modelParts[0].material.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//sphereModel.modelParts[0].material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//sphereModel.modelParts[0].material.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	//sphereModel.modelParts[0].material.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//sphereModel.modelParts[0].texDiffuse = SunTex;
	//m_pSun->SetModel(std::move(sphereModel));


	// ******************
	// 初始化阴影贴图和特效
	m_pShadowMap = std::make_unique<TextureRender>();
	HR(m_pShadowMap->InitResource(m_pd3dDevice.Get(), 2048, 2048, true));
	m_pGrayShadowMap = std::make_unique<TextureRender>();
	HR(m_pGrayShadowMap->InitResource(m_pd3dDevice.Get(), 512, 512));

	// 开启纹理、阴影
	m_pBasicEffect->SetTextureUsed(true);
	m_pBasicEffect->SetShadowEnabled(true);
	m_pBasicEffect->SetViewMatrix(camera->GetViewXM());
	m_pBasicEffect->SetProjMatrix(camera->GetProjXM());

	m_pShadowEffect->SetProjMatrix(XMMatrixOrthographicLH(40.0f, 40.0f, 20.0f, 60.0f));

	m_pDebugEffect->SetWorldMatrix(XMMatrixIdentity());
	m_pDebugEffect->SetViewMatrix(XMMatrixIdentity());
	m_pDebugEffect->SetProjMatrix(XMMatrixIdentity());

	// 调试用矩形
	Model quadModel;
	quadModel.SetMesh(m_pd3dDevice.Get(), Geometry::Create2DShow<VertexPosNormalTex>(XMFLOAT2(0.8125f, 0.6666666f), XMFLOAT2(0.1875f, 0.3333333f)));
	quadModel.modelParts[0].texDiffuse = m_pShadowMap->GetOutputTexture();
	m_DebugQuad.SetModel(std::move(quadModel));

	quadModel.SetMesh(m_pd3dDevice.Get(), Geometry::Create2DShow<VertexPosNormalTex>());
	quadModel.modelParts[0].texDiffuse = m_pShadowMap->GetOutputTexture();
	m_FullScreenDebugQuad.SetModel(std::move(quadModel));

	// ******************
	// 初始化天空盒相关

	m_pSkyBox = std::make_unique<SkyRender>();
	HR(m_pSkyBox->InitResource(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get(),
		L"..\\Texture\\Space.dds", 5000.0f));
	m_pSkyBox->SetSunInfo(*m_pSkyEffect, 20.0f, XMFLOAT3(1.0f, 1.0f, 1.0f));

	m_pBasicEffect->SetTextureCube(m_pSkyBox->GetTextureCube());

	// ******************
	// 初始化光照
	//
	m_DirLights[0].ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_DirLights[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 0.8f);
	m_DirLights[0].specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.7f);
	m_DirLights[0].direction = XMFLOAT3(5.0f / sqrtf(50.0f), -5.0f / sqrtf(50.0f), 0.0f);

	//m_DirLights[1].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//m_DirLights[1].diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	//m_DirLights[1].specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_DirLights[1].direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	//m_DirLights[2].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//m_DirLights[2].diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_DirLights[2].specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_DirLights[2].direction = XMFLOAT3(0.0f, 0.0, -1.0f);

	for (int i = 0; i < 3; ++i)
	{
		m_OriginalLightDirs[i] = m_DirLights[i].direction;
		m_pBasicEffect->SetDirLight(i, m_DirLights[i]);
	}

	// 开启纹理
	m_pBasicEffect->SetTextureUsed(true);


	// ******************
	// 设置调试对象名
	//
	m_pMainWorld->SetDebugObjectName("World");
	m_DebugQuad.SetDebugObjectName("DebugQuad");
	m_pShadowMap->SetDebugObjectName("ShadowMap");
	m_pSkyBox->SetDebugObjectName("Sky");

	return true;
}

void GameApp::UpdateCamera()
{
	// 获取子类
	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
	auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

	if (ImGui::Begin("Camera"))
	{

		static int curr_item = 0;
		static const char* modes[] = {
			"First Person",
			"Third Person",
		};
		if (ImGui::Combo("Camera Mode", &curr_item, modes, ARRAYSIZE(modes)))
		{
			if (curr_item == 0 && m_CameraMode != CameraMode::FirstPerson)
			{
				if (!cam1st)
				{
					cam1st.reset(new FirstPersonCamera);
					cam1st->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
					cam1st->SetFrustum(XM_PI / 2.5, AspectRatio(), 0.01f, 1000.0f);
					m_pCamera = cam1st;
				}

				m_CameraMode = CameraMode::FirstPerson;
			}
			else if (curr_item == 1 && m_CameraMode != CameraMode::ThirdPerson)
			{
				if (!cam3rd)
				{
					cam3rd.reset(new ThirdPersonCamera);
					cam3rd->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
					cam3rd->SetFrustum(XM_PI / 2.5, AspectRatio(), 0.01f, 1000.0f);
					m_pCamera = cam3rd;
				}
				XMFLOAT3 target = m_pPlayer->GetPosition();
				cam3rd->SetTarget(target);
				cam3rd->SetDistance(3.0f);
				cam3rd->SetDistanceMinMax(3.0f, 10.0f);

				m_CameraMode = CameraMode::ThirdPerson;
			}
			else if (curr_item == 2 && m_CameraMode != CameraMode::Free)
			{
				if (!cam1st)
				{
					cam1st.reset(new FirstPersonCamera);
					cam1st->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
					cam1st->SetFrustum(XM_PI / 2.5, AspectRatio(), 0.01f, 1000.0f);
					m_pCamera = cam1st;
				}
				//// 从箱子上方开始
				//XMFLOAT3 pos = woodCrateTransform.GetPosition();
				//XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
				//XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
				//pos.y += 3;
				//cam1st->LookTo(pos, to, up);

				m_CameraMode = CameraMode::Free;
			}
		}
	}
	ImGui::End();
}

