#ifndef __GAME_APP_H_
#define __GAME_APP_H_

#include "d3dApp.h"

class GameApp : public D3DApp {
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowTitle, int width, int height);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
};

#endif // __GAME_APP_H_
