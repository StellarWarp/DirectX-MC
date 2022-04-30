#include "PlayerController.h"

void PlayerController::UpdateCameraState(CameraMode newMode)
{
	static CameraMode mode = CameraMode::FirstPerson;
	if (newMode != mode)
	{
		mode = newMode;
		if (mode == CameraMode::FirstPerson)
		{
			m_pPlayer->FirstPersonModel();
		}
		else
		{
			m_pPlayer->ThirdPersonModel();
		}
	}
}



bool PlayerController::init(Player* player, ID3D11Device* device, std::shared_ptr<Camera>* camera, World* world)
{
	m_FPSCameraController.InitCamera(dynamic_cast<FirstPersonCamera*>(camera->get()));
	m_FPSCameraController.SetMoveSpeed(10.0f);
	m_FPSCameraController.SetStrafeSpeed(10.0f);

	m_pMouse = std::make_unique<DirectX::Mouse>();

	m_pPlayer = player;
	m_pWorld = world;
	m_pCamera = camera;
	m_pDevice = device;
	int num = 1;
	m_pPlayer->ObtainItem(InventoryItem::Grassland, &num); num = 1;
	m_pPlayer->ObtainItem(InventoryItem::Wood, &num); num = 1;
	m_pPlayer->ObtainItem(InventoryItem::Tree, &num); num = 1;
	m_pPlayer->ObtainItem(InventoryItem::Stone, &num); num = 1;
	m_pPlayer->ObtainItem(InventoryItem::RedBrick, &num); num = 1;
	m_pPlayer->ObtainItem(InventoryItem::Glass, &num);

	m_pPlayer->BirthPlace = XMFLOAT3(0, 70, 0);
	return true;
}

void PlayerController::Update(float dt)
{



	ImGuiIO& io = ImGui::GetIO();

	//移动
	// 获取子类
	auto& camera = *m_pCamera->get();
	auto cam1st = dynamic_cast<FirstPersonCamera*>(m_pCamera->get());
	auto cam3rd = dynamic_cast<ThirdPersonCamera*>(m_pCamera->get());

	//渲染中心
	m_pWorld->SetRenderCenter(m_pPlayer->GetPosition(), camera.GetPosition(), camera.GetLookAxis());

	float yaw = 0.0f, pitch = 0.0f, forward = 0.0f, strafe = 0.0f;

	if (MouseControl)
	{

		//Mouse::State mouseState = m_pMouse->GetState();
		//mouseState.positionMode = Mouse::MODE_RELATIVE;

		yaw += io.MouseDelta.x * m_MouseSensitivityX;
		pitch += io.MouseDelta.y * m_MouseSensitivityY;
	}
	else
	{
		yaw -= io.MouseWheelH * m_MouseSensitivityX * 20;
		pitch += io.MouseWheel * m_MouseSensitivityY * 20;
	}

	forward = (
		(ImGui::IsKeyDown('W') ? dt : 0.0f) +
		(ImGui::IsKeyDown('S') ? -dt : 0.0f)
		);
	strafe = (
		(ImGui::IsKeyDown('A') ? -dt : 0.0f) +
		(ImGui::IsKeyDown('D') ? dt : 0.0f)
		);

	XMFLOAT3 dir;
	XMStoreFloat3(&dir, camera.GetLookAxisXM() * forward + camera.GetRightAxisXM() * strafe);
	dir.y = 0;
	m_pPlayer->Move(dir, dt);

	if (ImGui::IsKeyDown(' '))
	{
		m_pPlayer->Jump(dt);
	}
	m_pPlayer->Update(dt);
	if (m_pPlayer->shouldRemove)
	{
		m_pPlayer->Death();
	}

	auto cpos = m_pPlayer->GetPositionWithOffset();
	cpos.y += 1.5;
	float roateX = 0;
	float roateY = 0;
	if (cam1st)
	{
		cam1st->RotateY(yaw);
		cam1st->Pitch(pitch);

		cam1st->SetPosition(cpos);
		roateX = cam1st->GetRotationX();
		roateY = cam1st->GetRotationY();
		UpdateCameraState(CameraMode::FirstPerson);
	}
	else if (cam3rd)
	{
		cam3rd->SetTarget(cpos);
		cam3rd->RotateX(pitch);
		cam3rd->RotateY(yaw);
		roateX = cam3rd->GetRotationX();
		roateY = cam3rd->GetRotationY();
		UpdateCameraState(CameraMode::ThirdPerson);
	}

	//生物扫描
	m_pPlayer->ScanCreature();

	//动画
	if (ImGui::IsKeyDown('W'))
		m_pPlayer->WalkFoward();
	if (ImGui::IsKeyDown('S'))
		m_pPlayer->WalkBackward();
	if (ImGui::IsKeyDown('A'))
		m_pPlayer->StrafeLeft();
	if (ImGui::IsKeyDown('D'))
		m_pPlayer->StrafeRight();

	m_pPlayer->RoateBody(roateY);
	m_pPlayer->RoateHead(roateX, 0);

	//储物
	for (UINT i = 0; i < 9; i++)
	{
		if (ImGui::IsKeyDown('1' + i))
		{
			m_pPlayer->SelectSort(i);
		}
	}
	if (ImGui::IsKeyDown('0'))
	{
		m_pPlayer->SelectSort(9);
	}
	Ray ray;
	if (!MouseControl)
		ray = Ray::ScreenToRay(camera, io.MousePos.x, io.MousePos.y);
	else
	{
		auto port = camera.GetViewPort();
		ray = Ray::ScreenToRay(camera, port.Width / 2, port.Height / 2);
	}

	//编辑
	//add
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		m_pPlayer->Interact();
		Block* target = nullptr;
		bool first = true;
		if (cam3rd)first = false;
		m_pWorld->OperateBlock(m_pDevice, ray, m_pPlayer->GetBlockType(), target, m_pPlayer->GetPosition(), first);
		if (target)
		{
			for (auto target_creature_id : (*target->objID))
			{
				auto target_creature = dynamic_cast<Creature*>(m_pPlayer->Objects[target_creature_id]);
				m_pPlayer->Attack(target_creature, dt);
			}
		}
	}
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		m_pPlayer->Interact();
		m_pWorld->RemoveBlock(ray, m_pPlayer->GetPosition());
	}

	m_pPlayer->UpdateAnimation(dt);

	static std::string itemname[] = {
		"Null",
		"Land",
		"Sand",
		"Stone",
		"RedBrick",
		"Wood",
		"Grassland",
		"Tree",
		"Snow",
		"LeaveGreen",
		"LeaveRed",
		"LeaveYellow",
		"LeaveWhite",
		"Glass"
	};
	ImGui::Begin("PlayerInfo"); 
	ImGui::Text("Use number 0~9 to select sort");
	ImGui::Text(itemname[static_cast<int>(m_pPlayer->GetItem())].c_str()); 
	ImGui::Checkbox("Mouse Contorl Mode", &MouseControl);
	ImGui::End();

}