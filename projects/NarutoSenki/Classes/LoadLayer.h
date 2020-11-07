#pragma once
#include "Defines.h"
#include "GameLayer.h"
#include "BGLayer.h"
#include "HudLayer.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "../../../cocos2dx/platform/android/jni/JniHelper.h"
#endif

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

class LoadLayer : public CCLayer
{
public:
	LoadLayer();
	~LoadLayer();

	CCArray *tempHeros;
	bool init();

	//void dfsFolder(std::string folderPath, int depth);
	int loadNum;

	CREATE_FUNC(LoadLayer);

	CC_SYNTHESIZE_RETAIN(GameLayer *, _gameLayer, GameLayer);
	CC_SYNTHESIZE_RETAIN(BGLayer *, _bgLayer, BGLayer);
	CC_SYNTHESIZE_RETAIN(HudLayer *, _hudLayer, HudLayer);
	bool _isHardCoreMode;
	bool _isRandomChar;

	void preloadAudio();
	void preloadIMG();
	void playBGM(float dt);
	void onLoadFinish(float dt);

	bool isPosting;
	void onHttpRequestCompleted(CCHttpClient *client, CCHttpResponse *response);

	CCArray *DLCArray;

private:
	void preloadPlist(CCObject *obj);

protected:
	void onEnter();
	void onExit();
};
