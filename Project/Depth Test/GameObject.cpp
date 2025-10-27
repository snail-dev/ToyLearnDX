#include "GameObject.h"
#include "d3dUtil.h"

GameObject::GameObject() :
	m_IndexCount(),
	m_VertexStride(),
	m_Material()
{
}

Transform& GameObject::GetTransform()
{
	return m_Transform;
}

const Transform& GameObject::GetTransform() const
{
	return m_Transform;
}

void GameObject::SetTexture(ID3D11ShaderResourceView* texture)
{
	m_SRV = texture;
}

void GameObject::SetMaterial(const Material& material)
{
	m_Material = material;
}

void GameObject::Draw(ID3D11DeviceContext* context, BasicEffect& effect)
{
	UINT strides = m_VertexStride;
	UINT offsets = 0;

	context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	effect.SetWorldMatrix(m_Transform.GetLocalToWorldMatrixXM());
	effect.SetMaterial(m_Material);
	effect.SetSRV(m_SRV.Get());
	effect.Apply(context);

	context->DrawIndexed(m_IndexCount, 0, 0);
}

void GameObject::SetDebugObjectName(const std::string& name)
{
#if (defined (DEBUG) || defined (_DEBUG))  && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
#else
	UNREFERENCED_PARAMETER(name);
#endif
}




