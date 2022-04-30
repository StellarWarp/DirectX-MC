#include "CameraController.h"
#include "d3dUtil.h"
#include <imgui.h>

using namespace DirectX;
#undef min;
void CameraController::ApplyMomentum(float& oldValue, float& newValue, float deltaTime)
{
	deltaTime = std::min(1.0f / 100.0f, deltaTime);

	float blendedValue;
	if (fabs(newValue) > fabs(oldValue))
		blendedValue = XMath::Lerp(newValue, oldValue, powf(0.6f, deltaTime * 60.0f));
	else
		blendedValue = XMath::Lerp(newValue, oldValue, powf(0.8f, deltaTime * 60.0f));

	oldValue = blendedValue;
	newValue = blendedValue;
}

void FirstPersonCameraController::Update(float deltaTime)
{
	ImGuiIO& io = ImGui::GetIO();

	float yaw = 0.0f, pitch = 0.0f, forward = 0.0f, strafe = 0.0f, vertical = 0.0f;
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		yaw += io.MouseDelta.x * m_MouseSensitivityX;
		pitch += io.MouseDelta.y * m_MouseSensitivityY;
	}
	yaw -= io.MouseWheelH * m_MouseSensitivityX * 20;
	pitch += io.MouseWheel * m_MouseSensitivityY * 20;

	forward = m_MoveSpeed * (
		(ImGui::IsKeyDown('W') ? deltaTime : 0.0f) +
		(ImGui::IsKeyDown('S') ? -deltaTime : 0.0f)
		);
	strafe = m_StrafeSpeed * (
		(ImGui::IsKeyDown('A') ? -deltaTime : 0.0f) +
		(ImGui::IsKeyDown('D') ? deltaTime : 0.0f)
		);

	vertical = m_StrafeSpeed * (
		(ImGui::IsKeyDown('F') ? -deltaTime : 0.0f) +
		(ImGui::IsKeyDown('R') ? deltaTime : 0.0f)
		);

	if (m_Momentum)
	{
		ApplyMomentum(m_LastForward, forward, deltaTime);
		ApplyMomentum(m_LastStrafe, strafe, deltaTime);
		ApplyMomentum(m_LastVertical, vertical, deltaTime);
	}

	m_pCamera->RotateY(yaw);
	m_pCamera->Pitch(pitch);

	m_pCamera->MoveForward(forward);
	m_pCamera->Strafe(strafe);
	m_pCamera->MoveUp(vertical);
}

void FirstPersonCameraController::InitCamera(FirstPersonCamera* pCamera)
{
	m_pCamera = pCamera;
}

void FirstPersonCameraController::SlowMovement(bool enable)
{
	m_FineMovement = enable;
}

void FirstPersonCameraController::SlowRotation(bool enable)
{
	m_FineRotation = enable;
}

void FirstPersonCameraController::EnableMomentum(bool enable)
{
	m_Momentum = enable;
}

void FirstPersonCameraController::SetMouseSensitivity(float x, float y)
{
	m_MouseSensitivityX = x;
	m_MouseSensitivityY = y;
}

void FirstPersonCameraController::SetMoveSpeed(float m_speed)
{
	m_MoveSpeed = m_speed;
}

void FirstPersonCameraController::SetStrafeSpeed(float m_speed)
{
	m_StrafeSpeed = m_speed;
}
