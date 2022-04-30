#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include "d3dApp.h"
#include"CameraController.h"
#include "ObjReader.h"
#include "Collision.h"
#include "SkyRender.h"
#include "TextureRender.h"
#include "PlayerController.h"
#include "World.h"
#include"Animal.h"

class GameApp : public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
	void DrawScene(BasicEffect* pBasicEffect, bool enableNormalMap = false);
	void DrawScene(ShadowEffect* pShadowEffect);

private:
	bool InitResource();

	void UpdateCamera();

private:
	
	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;				    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;								// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;					// 文本格式

	bool m_EnableNormalMap;										// 开启法线贴图
	bool m_EnableDebug;											// 开启调试模式
	bool m_GrayMode;											// 深度值以灰度形式显示
	int m_SlopeIndex;			     							// 斜率索引
	bool m_EnableClip;                                //视锥裁切


	GameObject m_DebugQuad;										// 调试用四边形
	GameObject m_FullScreenDebugQuad;							// 全屏调试用四边形

	DirectionalLight m_DirLights[3] = {};						// 方向光
	DirectX::XMFLOAT3 m_OriginalLightDirs[3] = {};				// 初始光方向

	std::unique_ptr<BasicEffect> m_pBasicEffect;				// 基础特效
	std::unique_ptr<ShadowEffect> m_pShadowEffect;				// 阴影特效
	std::unique_ptr<SkyEffect> m_pSkyEffect;					// 天空盒特效
	std::unique_ptr<DebugEffect> m_pDebugEffect;				// 调试用显示纹理的特效

	std::unique_ptr<TextureRender> m_pShadowMap;				// 阴影贴图
	std::unique_ptr<TextureRender> m_pGrayShadowMap;			// 用于调试的RGB阴影贴图
	std::unique_ptr<SkyRender> m_pSkyBox;						// 天空盒
	
	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_CameraMode;									// 摄像机模式

	std::shared_ptr<Player> m_pPlayer;
	PlayerController m_Controller;
	std::vector<std::shared_ptr<Animal>> m_pAnimals;
	//std::shared_ptr<GameObject> m_pSun;

	std::shared_ptr<World> m_pMainWorld;
};


#endif