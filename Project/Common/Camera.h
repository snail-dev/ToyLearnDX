#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Transform.h"
#include <d3d11.h>

class Camera {
public:
	Camera() = default;
	virtual ~Camera() = default;

	DirectX::XMVECTOR GetPositionXM() const;
	DirectX::XMFLOAT3 GetPosition() const;

	float GetRotationX() const;
	float GetRotationY() const;

	DirectX::XMVECTOR GetRightAxisXM() const;
	DirectX::XMFLOAT3 GetRightAxis() const;
	DirectX::XMVECTOR GetUpAxisXM() const;
	DirectX::XMFLOAT3 GetUpAxis() const;
	DirectX::XMVECTOR GetLookAxisXM() const;
	DirectX::XMFLOAT3 GetLookAxis() const;

	DirectX::XMMATRIX GetViewXM() const;
	DirectX::XMMATRIX GetProjXM() const;
	DirectX::XMMATRIX GetViewProjXM() const;

	D3D11_VIEWPORT GetViewPort() const;

	void SetFrustum(float fovY, float aspect, float nearZ, float farZ);
	void SetViewPort(const D3D11_VIEWPORT& viewPort);
	void SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth=0.0f, float maxDepth = 1.0f);

protected:
	Transform m_Transform = {};

	float m_NearZ = 0.0f;
	float m_FarZ = 0.0f;
	float m_Aspect = 0.0f;
	float m_FovY = 0.0f;

	D3D11_VIEWPORT m_ViewPort = {};
};

class FirstPersonCamera : public Camera {
public:
	FirstPersonCamera() = default;
	virtual ~FirstPersonCamera();

	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& pos);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);
	void LookTo(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& to, const DirectX::XMFLOAT3& up);	
	void Strafe(float d);
	void Walk(float d);
	void MoveForward(float d);
	void Pitch(float rad);
	void RotateY(float rad);
};

class ThirdPersonCamera : public Camera {
public:
	ThirdPersonCamera() = default;
	~ThirdPersonCamera();

	DirectX::XMFLOAT3 GetTargetPosition() const;
	float GetDistance() const;
	void RotateX(float rad);
	void RotateY(float rad);
	void Approach(float dist);

	void SetRotationX(float rad);
	void SetRotationY(float rad);
	void SetTarget(const DirectX::XMFLOAT3& target);
	void SetDistance(float dist);
	void SetDistanceMinMax(float minDist, float maxDist);
	
private:
	DirectX::XMFLOAT3 m_Target = {};
	float m_Distance = 0.0f;
	float m_MinDist = 0.0f;
	float m_MaxDist = 0.0f;
};

#endif // _CAMERA_H_