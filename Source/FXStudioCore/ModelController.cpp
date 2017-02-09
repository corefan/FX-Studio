#include "stdafx.h"
#include "ModelController.h"
#include "FXStudioEvent.h"

ModelController::ModelController(shared_ptr<CameraNode> pEditorCamera, shared_ptr<DebugGizmosNode> pGizmosNode,
	const Vector3& cameraPos, float cameraYaw, float cameraPitch)
	: m_pEditorCamera(pEditorCamera),
	m_pGizmosNode(pGizmosNode),
	m_CameraType(CT_FirstPerson),
	m_Position(cameraPos),
	m_Yaw(-cameraYaw),
	m_Pitch(cameraPitch),
	m_TargetYaw(m_Yaw),
	m_TargetPitch(m_Pitch),
	m_Delta(0.0f),
	m_MoveX(0.0f),
	m_MoveY(0.0f),
	m_CurrentSpeed(0.0f),
	m_MaxSpeed(50.0f),
	m_IsLButtonDown(false)
{
	POINT ptCursor;
	GetCursorPos(&ptCursor);
	m_LastMousePos.x = static_cast<float>(ptCursor.x);
	m_LastMousePos.y = static_cast<float>(ptCursor.y);

	memset(m_Keys, 0x00, sizeof(m_Keys));
}

void ModelController::OnUpdate(const GameTime& gameTime)
{
	switch (m_CameraType)
	{
	case CT_FirstPerson:
	{
		bool isTranslating = false;
		Vector3 forward = Vector3::Zero;
		Vector3 strafe = Vector3::Zero;

		Matrix worldMatrix = m_pEditorCamera->VGet()->GetWorldMatrix();
		if (m_Keys['W'] || m_Keys[VK_UP])
		{
			forward = Vector3::TransformNormal(Vector3::Forward, worldMatrix);
			isTranslating = true;
		}
		else if (m_Keys['S'] || m_Keys[VK_DOWN])
		{
			forward = Vector3::TransformNormal(Vector3::Backward, worldMatrix);
			isTranslating = true;
		}

		if (m_Keys['A'] || m_Keys[VK_LEFT])
		{
			strafe = Vector3::TransformNormal(Vector3::Right, worldMatrix);
			isTranslating = true;
		}
		else if (m_Keys['D'] || m_Keys[VK_RIGHT])
		{
			strafe = Vector3::TransformNormal(Vector3::Left, worldMatrix);
			isTranslating = true;
		}

		if (isTranslating)
		{
			float numberOfSeconds = 5.0f;
			m_CurrentSpeed += m_MaxSpeed * ((gameTime.GetElapsedTime() * gameTime.GetElapsedTime()) / numberOfSeconds);
			if (m_CurrentSpeed > m_MaxSpeed)
				m_CurrentSpeed = m_MaxSpeed;

			Vector3 direction = forward + strafe;
			direction.Normalize();
			direction *= m_CurrentSpeed;
			m_Position += direction;
		}
		else
		{
			m_CurrentSpeed = 0.0f;
		}

		{
			m_Yaw += (m_TargetYaw - m_Yaw) * (.35f);
			m_TargetPitch = std::max<float>(-90.0f, std::min<float>(90.0f, m_TargetPitch));
			m_Pitch += (m_TargetPitch - m_Pitch) * (.35f);

			Matrix rotation = Matrix::CreateFromYawPitchRoll(XMConvertToRadians(m_Yaw), XMConvertToRadians(m_Pitch), 0.0f);
			rotation.Translation(m_Position);

			if (m_pEditorCamera != nullptr)
			{
				m_pEditorCamera->VSetTransform(rotation);
			}
		}

		break;
	}
	case CT_OrbitView:
	{
		float speed = m_Delta * 0.01f;
		m_Position += Vector3::Forward * speed;
		m_Delta = 0.0f;

		m_Yaw += (m_TargetYaw - m_Yaw) * (.35f);
		m_Pitch += (m_TargetPitch - m_Pitch) * (.35f);

		float radiansYaw = XMConvertToRadians(m_Yaw);
		float radiansPitch = -XMConvertToRadians(m_Pitch);
		Matrix rotation = Matrix::CreateFromYawPitchRoll(radiansYaw, radiansPitch, 0.0f);
		rotation.Translation(Vector3::TransformNormal(m_Position, rotation));

		if (m_pEditorCamera != nullptr)
		{
			m_pEditorCamera->VSetTransform(rotation);
		}
		break;
	}
	default:
		break;
	}

	shared_ptr<EvtData_Move_Camera> pEvent(DEBUG_NEW EvtData_Move_Camera());
	IEventManager::Get()->VQueueEvent(pEvent);
}

bool ModelController::VOnPointerLeftButtonDown(const Vector2 &pos, int radius)
{
	m_IsLButtonDown = true;
	m_LastMousePos = pos;

	if (m_pGizmosNode != nullptr)
	{
		m_pGizmosNode->PointerLeftClick(pos);
	}

	return true;
}

bool ModelController::VOnPointerLeftButtonUp(const Vector2 &pos, int radius)
{
	m_IsLButtonDown = false;

	if (m_pGizmosNode != nullptr)
	{
		m_pGizmosNode->PointerMove(pos, m_IsLButtonDown);
	}
	return true;
}

bool ModelController::VOnPointerRightButtonDown(const Vector2 &pos, int radius)
{
	return true;
}

bool ModelController::VOnPointerRightButtonUp(const Vector2 &pos, int radius)
{
	return true;
}

bool ModelController::VOnPointerMove(const Vector2 &pos, int radius)
{
	if (m_LastMousePos != pos)
	{
		switch (m_CameraType)
		{
		case CT_FirstPerson:
		{
			if (m_IsLButtonDown)
			{
				m_TargetYaw += (m_LastMousePos.x - pos.x);
				m_TargetPitch += (pos.y - m_LastMousePos.y);
			}

			break;
		}
		case CT_OrbitView:
		{
			if (m_IsLButtonDown)
			{
				if (GetKeyState(VK_MENU) < 0)
				{
					m_TargetYaw += (m_LastMousePos.x - pos.x);
					m_TargetPitch += (pos.y - m_LastMousePos.y);
				}
				else if (m_Keys[VK_SHIFT])
				{
					m_Delta = (m_LastMousePos.y - pos.y);
				}
				else
				{
					if (m_pGizmosNode != nullptr)
					{
						m_pGizmosNode->PointerMove(pos, m_IsLButtonDown);
					}
				}
			}
			else
			{
				if (m_pGizmosNode != nullptr)
				{
					m_pGizmosNode->PointerMove(pos, m_IsLButtonDown);
				}
			}

			break;
		}
		default:
			break;
		}

		m_LastMousePos = pos;
	}

	return true;
}

bool ModelController::VOnPointerWheel(int16_t delta)
{
	m_Delta = static_cast<float>(delta);

	return true;
}

