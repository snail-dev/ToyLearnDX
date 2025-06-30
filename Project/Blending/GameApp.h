#ifndef __GAME_APP_H_
#define __GAME_APP_H_

#include "d3dApp.h"
#include "d3dUtil.h"
#include "LightHelper.h"
#include "Vertex.h"
#include "Geometry.h"
#include "Camera.h"

#include <DirectXMath.h>
#include <memory>


class GameApp : public D3DApp {
public:

	struct CBChangesEveryDrawing {
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
		Material material;
	};

	struct CBChangesEveryFrame {
		DirectX::XMMATRIX view;
		DirectX::XMVECTOR eyePos;
	};

	struct CBChangesOnResize {
		DirectX::XMMATRIX proj;
	};

	struct CBChangesRelaly {
		DirectionalLight dirLight[10];
		PointLight pointLight[10];
		SpotLight spotLight[10];
		int numDirLight;
		int numPointLight;
		int numSpotLight;
		float pad;
	};


	class GameObject {
	public:
		GameObject();
		~GameObject();

		Transform& GetTransform();
		const Transform& GetTransform() const;

		template <typename VertexType, typename IndexType>
		void SetBuffer(ID3D11Device* device,  const Geometry::MeshData<VertexType, IndexType>& meshData);

		void SetTexture(ID3D11ShaderResourceView* text);
		void SetMaterial(const Material& material);
		void Draw(ID3D11DeviceContext* context);
		void SetDebugObjectName(const std::string& name);
	private:
		Transform m_Transform;
		Material m_Material;
		ComPtr<ID3D11ShaderResourceView> m_SRV;
		ComPtr<ID3D11Buffer> m_pVertexBuffer;
		ComPtr<ID3D11Buffer> m_pIndexBuffer;

		UINT m_VertexStride;
		UINT m_IndexCount;
	};

	enum class CameraMode {FirstPerson, ThirdPerson, Free};
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


private:
    bool InitEffect();
	bool InitResource();

	template <typename VertexType>
	bool ResetMesh(const Geometry::MeshData<VertexType>& meshData);

private:
    ComPtr<ID3D11InputLayout> m_pVertexLayout2D;
	ComPtr<ID3D11InputLayout> m_pVertexLayout3D;

    ComPtr<ID3D11Buffer> m_pConstantBuffer[4];

	GameObject m_WireFence;
	GameObject m_Floor;
	std::vector<GameObject> m_Walls;
	GameObject m_Water;	

    ComPtr<ID3D11VertexShader> m_pVertexShader3D;
	ComPtr<ID3D11PixelShader> m_pPixelShader3D;
	ComPtr<ID3D11VertexShader> m_pVertexShader2D;
	ComPtr<ID3D11PixelShader> m_pPixelShader2D;

	CBChangesEveryFrame m_CBFrame;
	CBChangesOnResize m_CBResize;
	CBChangesRelaly m_CBRarely;

	std::shared_ptr<Camera> m_pCamera;
	CameraMode m_CameraMode;
};

#endif // __GAME_APP_H_
