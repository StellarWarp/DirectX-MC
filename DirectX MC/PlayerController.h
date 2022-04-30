#pragma once
#include"Player.h"
#include"CameraController.h"
#include "World.h"
#include <d3dApp.h>
class PlayerController
{
	FirstPersonCameraController m_FPSCameraController;				// 摄像机控制器
	Player* m_pPlayer;
	World* m_pWorld;
	std::shared_ptr<Camera>* m_pCamera;
	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	ID3D11Device* m_pDevice;

	// 键鼠输入
	std::unique_ptr<DirectX::Mouse> m_pMouse;						// 鼠标
	DirectX::Mouse::ButtonStateTracker m_MouseTracker;				// 鼠标状态追踪器
	bool  MouseControl = false;

	float m_MouseSensitivityX = 0.005f;
	float m_MouseSensitivityY = 0.005f;


	void UpdateCameraState(CameraMode newMode);

public:
	bool init(Player* player, ID3D11Device* device, std::shared_ptr<Camera>* camera, World* world);
	
	void Update(float dt);
};

