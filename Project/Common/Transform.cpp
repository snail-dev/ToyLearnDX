#include "Transform.h"

using namespace DirectX;

Transform::Transform(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& position) :
	 m_Scale(scale),
	 m_Rotation(rotation),
	 m_Position(position) 
{

}
	
DirectX::XMFLOAT3 Transform::GetScale() const {
	return m_Scale;
}

DirectX::XMVECTOR Transform::GetScaleXM() const {
	return DirectX::XMLoadFloat3(&m_Scale);
}

DirectX::XMFLOAT3 Transform::GetRotation() const {
	return m_Rotation;
}
DirectX::XMVECTOR Transform::GetRotationXM() const {
	return DirectX::XMLoadFloat3(&m_Rotation);
} 

DirectX::XMFLOAT3 Transform::GetPosition() const {
	return m_Position;
}
DirectX::XMVECTOR Transform::GetPositionXM() const {
	return DirectX::XMLoadFloat3(&m_Position);
}

DirectX::XMFLOAT3 Transform::GetRightAxis() const {
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&m_Rotation));
    DirectX::XMFLOAT3 right;
    DirectX::XMStoreFloat3(&right, R.r[0]);
    return right;
}
DirectX::XMVECTOR Transform::GetRightAxisXM() const {
	DirectX::XMFLOAT3 right = GetRightAxis();
	return DirectX::XMLoadFloat3(&right);	
}

DirectX::XMFLOAT3 Transform::GetUpAxis() const {
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&m_Rotation));
	DirectX::XMFLOAT3 up;
	DirectX::XMStoreFloat3(&up, R.r[1]);
	return up;
}
DirectX::XMVECTOR Transform::GetUpAxisXM() const {
	DirectX::XMFLOAT3 up = GetUpAxis();
	return DirectX::XMLoadFloat3(&up);
}

DirectX::XMFLOAT3 Transform::GetForwardAxis() const {
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&m_Rotation));
	DirectX::XMFLOAT3 forward;
	DirectX::XMStoreFloat3(&forward, R.r[2]);
	return forward;
}
DirectX::XMVECTOR Transform::GetForwardAxisXM() const {
	DirectX::XMFLOAT3 forward = GetForwardAxis();
	return DirectX::XMLoadFloat3(&forward);
}

DirectX::XMFLOAT4X4 Transform::GetLocalToWorldMatrix() const {
	DirectX::XMFLOAT4X4 res;
	DirectX::XMStoreFloat4x4(&res, GetLocalToWorldMatrixXM());
	return res;
}
DirectX::XMMATRIX Transform::GetLocalToWorldMatrixXM() const {
	DirectX::XMVECTOR scaleVec = DirectX::XMLoadFloat3(&m_Scale);
	DirectX::XMVECTOR rotationVec = DirectX::XMLoadFloat3(&m_Rotation);
	DirectX::XMVECTOR positionVec = DirectX::XMLoadFloat3(&m_Position);
	DirectX::XMMATRIX World = DirectX::XMMatrixScalingFromVector(scaleVec) *
	                          DirectX::XMMatrixRotationRollPitchYawFromVector(rotationVec) *
							  DirectX::XMMatrixTranslationFromVector(positionVec);

	return World;
}

DirectX::XMFLOAT4X4 Transform::GetWorldToLocalMatrix() const {
	DirectX::XMFLOAT4X4 res;
	DirectX::XMStoreFloat4x4(&res, GetWorldToLocalMatrixXM());
	return res;
}

DirectX::XMMATRIX Transform::GetWorldToLocalMatrixXM() const {
	DirectX::XMMATRIX InvWorld = XMMatrixInverse(nullptr, GetLocalToWorldMatrixXM());
	return InvWorld;
}

void Transform::SetScale(const DirectX::XMFLOAT3& scale) {
	m_Scale = scale;
}
void Transform::SetScale(float x, float y, float z) {
	m_Scale = DirectX::XMFLOAT3(x,y,z);
}

void Transform::SetRotation(const DirectX::XMFLOAT3& rotation) {
	m_Rotation = rotation;
}
void Transform::SetRotation(float x, float y, float z) {
	m_Rotation = DirectX::XMFLOAT3(x,y,z);
}

void Transform::SetPosition(const DirectX::XMFLOAT3& position) {
	m_Position = position;	
}
void Transform::SetPosition(float x, float y, float z) {
	m_Position = DirectX::XMFLOAT3(x,y,z);
}

void Transform::Rotate(const DirectX::XMFLOAT3& angle) {
	DirectX::XMVECTOR newAngle = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_Rotation), DirectX::XMLoadFloat3(&angle));
}

void Transform::RotateAxis(const DirectX::XMFLOAT3& axis, float radian) {
	DirectX::XMVECTOR rotationVec = DirectX::XMLoadFloat3(&m_Rotation);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYawFromVector(rotationVec) * DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&axis),radian);

	DirectX::XMFLOAT4X4 rotMatirx;
	DirectX::XMStoreFloat4x4(&rotMatirx, R);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatirx);
}
void Transform::RotateAround(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT3& axis, float radian) {
	DirectX::XMVECTOR rotationVec = DirectX::XMLoadFloat3(&m_Rotation);
	DirectX::XMVECTOR positionVec = DirectX::XMLoadFloat3(&m_Position);
	DirectX::XMVECTOR centerVec = DirectX::XMLoadFloat3(&point);
	
	DirectX::XMMATRIX RT = DirectX::XMMatrixRotationRollPitchYawFromVector(rotationVec) * DirectX::XMMatrixTranslationFromVector(positionVec - centerVec);

	RT *= DirectX::XMMatrixRotationAxis(DirectX::XMLoadFloat3(&axis),radian);
	RT *= DirectX::XMMatrixTranslationFromVector(centerVec);

	DirectX::XMFLOAT4X4 rotMatirx;
	DirectX::XMStoreFloat4x4(&rotMatirx, RT);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatirx);
	DirectX::XMStoreFloat3(&m_Position, RT.r[3]);
}

void Transform::Translate(const DirectX::XMFLOAT3& direction, float magnitude) {
	DirectX::XMVECTOR directionVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&direction));
	DirectX::XMVECTOR newPosition = DirectX::XMVectorMultiplyAdd(DirectX::XMVectorReplicate(magnitude), directionVec, DirectX::XMLoadFloat3(&m_Position));
	DirectX::XMStoreFloat3(&m_Position, newPosition);
}

void Transform::LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up) {
	DirectX::XMMATRIX View = XMMatrixLookAtLH(XMLoadFloat3(&m_Position), XMLoadFloat3(&target), XMLoadFloat3(&up));
	XMMATRIX InView = XMMatrixInverse(nullptr,View);
	XMFLOAT4X4 rotMatrix;
	XMStoreFloat4x4(&rotMatrix, InView);
	m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
}

void Transform::LookTo(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up ) {
    XMMATRIX View = XMMatrixLookToLH(XMLoadFloat3(&m_Position), XMLoadFloat3(&direction), XMLoadFloat3(&up));
    XMMATRIX InView = XMMatrixInverse(nullptr, View);
    XMFLOAT4X4 rotMatrix;
    XMStoreFloat4x4(&rotMatrix, InView);
    m_Rotation = GetEulerAnglesFromRotationMatrix(rotMatrix);
} 

DirectX::XMFLOAT3 Transform::GetEulerAnglesFromRotationMatrix(const DirectX::XMFLOAT4X4& rotationMatrix) {
	DirectX::XMFLOAT3 rotation;

	if(fabs(1.0f - fabs(rotationMatrix(2,1))) < 1e-5f) {
		rotation.x = copysignf(DirectX::XM_PIDIV2, -rotationMatrix(2,1));
		rotation.y = -atan2f(rotationMatrix(0,2), rotationMatrix(0,0));

		return rotation;
	}

	float c = sqrtf(1.0f - rotationMatrix(2,1) * rotationMatrix(2,1));

	if(isnan(c)) 
		c = 0.0f;

	rotation.z = atan2f(rotationMatrix(0,1), rotationMatrix(1,1));
	rotation.x = atan2f(-rotationMatrix(2,1),c);
	rotation.y = atan2f(rotationMatrix(2,0), rotationMatrix(2,2));

	return rotation;
}
