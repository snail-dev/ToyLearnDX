#ifndef __GAME_APP_H_
#define __GAME_APP_H_

#include "d3dApp.h"
#include "d3dUtil.h"
#include "GameObject.h"
#include "Camera.h"
#include <DirectXMath.h>
#include <memory>


class GameApp : public D3DApp {
public:
	enum class CameraMode {FirstPerson, ThirdPerson, Free};
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


private:
	bool InitResource();
private:
   
	GameObject m_WoodCrate;
	GameObject m_Floor;
	std::vector<GameObject> m_Walls;
	GameObject m_Mirror;

	Material m_ShadowMat;
	Material m_WoodCrateMat;

	BasicEffect  m_BasicEffect;

	std::shared_ptr<Camera> m_pCamera;
	CameraMode m_CameraMode;
};

#endif // __GAME_APP_H_
