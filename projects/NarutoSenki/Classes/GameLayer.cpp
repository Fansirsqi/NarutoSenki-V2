#include "Defines.h"
#include "CharacterBase.h"
#include "GameLayer.h"
#include "BGLayer.h"
#include "HudLayer.h"
#include "StartMenu.h"
#include "Core/Provider.hpp"
#include "GameMode/GameModeImpl.h"

GameLayer *_gLayer = nullptr;
bool _isFullScreen = false;

GameLayer::GameLayer()
{
	mapId = 0;

	_isAttackButtonRelease = true;
	_isSkillFinish = true;

	_second = 0;
	_minute = 0;
	_playNum = 2;

	kEXPBound = 25;
	aEXPBound = 25;

	totalKills = nullptr;
	totalTM = nullptr;
	_isShacking = false;
	_isSurrender = false;
	_hasSpawnedGuardian = false;

	_isStarted = false;
	_isExiting = false;

	ougisChar = nullptr;
	controlChar = nullptr;

	_enableGear = true;
	_isOugis2Game = false;
	_isHardCoreGame = false;
	_isRandomChar = false;

	currentPlayer = nullptr;

	_isGear = false;
	_isPause = false;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	_lastPressedMovementKey = -100;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	_lastPressedMovementKey = -100;
	_window = CCEGLView::sharedOpenGLView()->m_window;
#endif
}

GameLayer::~GameLayer()
{
	_gLayer = nullptr;
	removeKeyEventHandler();
}

bool GameLayer::init()
{
	CCTexture2D::setDefaultAlphaPixelFormat(kCCTexture2DPixelFormat_RGBA8888);
	setTouchEnabled(true);

	_gLayer = this;
	const auto &gd = getGameModeHandler()->gd;
	_enableGear = gd.enableGear;
	_isHardCoreGame = gd.isHardCore;
	_isRandomChar = gd.isRandomChar;
	playerGroup = gd.playerGroup;

	return CCLayer::init();
}

void GameLayer::onEnter()
{
	if (_isExiting)
	{
		onLeft();
		return;
	}

	if (currentPlayer && !ougisChar)
	{
		if (currentPlayer->getActionState() == State::WALK)
		{
			currentPlayer->idle();
		}
	}

	CCLayer::onEnter();

	if (_isSurrender)
	{
		onGameOver(false);
	}
}

void GameLayer::onExit()
{
	CCLayer::onExit();

	if (_isExiting)
	{
		_isExiting = false;
		_TowerArray.clear();
		_KonohaFlogArray.clear();
		_AkatsukiFlogArray.clear();
		_CharacterArray.clear();
	}
}

void GameLayer::onHUDInitialized(const OnHUDInitializedCallback &callback)
{
	callbackssList.push_back(callback);
}

bool GameLayer::isHUDInit()
{
	return isHUDInitialized;
}

void GameLayer::initTileMap()
{
	setRand();
	int mapCount = getMapCount();
	mapId = random(mapCount) + 1;
	currentMap = CCTMXTiledMap::create(GetMapPath(mapId));
	addChild(currentMap, currentMapTag);
}

void GameLayer::initGard()
{
	setRand();
	int index = random(2);
	auto guardianName = index == 0 ? kGuardian_Roshi : kGuardian_Han;
	auto guardianGroup = playerGroup == Konoha ? Akatsuki : Konoha;
	auto guardian = Provider::create(CCString::create(guardianName), CCString::create(kRoleCom), CCString::create(guardianGroup));

	if (playerGroup == Konoha)
	{
		guardian->setPosition(ccp(2800, 80));
		guardian->setSpawnPoint(ccp(2800, 80));
	}
	else
	{
		guardian->setPosition(ccp(272, 80));
		guardian->setSpawnPoint(ccp(272, 80));
	}

	addChild(guardian, -guardian->getPositionY());
	guardian->setLV(6);
	guardian->setHPbar();
	guardian->setShadows();
	guardian->setCharNO(_CharacterArray.size() + 1);

	guardian->idle();
	auto callValue = CCDictionary::create();
	callValue->setObject(CCString::create("smk"), 1);
	guardian->setSkillEffect(guardian, callValue);

	guardian->doAI();

	_CharacterArray.push_back(guardian);
	_hudLayer->addMapIcon();

	_hasSpawnedGuardian = true;
}

void GameLayer::initHeros()
{
	initTileMap();
	initEffects();

	addSprites("Element/hpBar/hpBar.plist");

	auto handler = getGameModeHandler();
	auto herosDataVector = handler->getHerosArray();

	_isOugis2Game = true;

	int i = 0;
	CCTMXObjectGroup *group = currentMap->objectGroupNamed("object");
	CCArray *objectArray = group->getObjects();

	// 4v4
	if (Cheats >= MaxCheats)
	{
		auto &hero1 = herosDataVector.at(0);
		auto &hero5 = herosDataVector.at(4);

		spawnPoint = getCustomSpawnPoint(hero1);
		auto hero = addHero(CCString::create(hero1.character), CCString::create(hero1.role), CCString::create(hero1.group), spawnPoint, 1);
		_CharacterArray.push_back(hero);

		spawnPoint = getCustomSpawnPoint(hero5);
		hero = addHero(CCString::create(hero5.character), CCString::create(hero5.role), CCString::create(hero5.group), spawnPoint, 5);
		_CharacterArray.push_back(hero);
	}

	for (const auto &data : herosDataVector)
	{
		if (data.isInit)
			continue;

		int mapPos = i;
		if (data.group == Akatsuki)
		{
			if (mapPos <= MapPosCount - 1)
				mapPos += MapPosCount;
		}
		else
		{
			if (mapPos > MapPosCount - 1)
				mapPos -= MapPosCount;
		}

		CCObject *mapObject = objectArray->objectAtIndex(mapPos);
		auto mapdict = (CCDictionary *)mapObject;
		int x = ((CCString *)mapdict->objectForKey("x"))->intValue();
		int y = ((CCString *)mapdict->objectForKey("y"))->intValue();
		spawnPoint = ccp(x, y);

		auto hero = addHero(CCString::create(data.character), CCString::create(data.role), CCString::create(data.group), spawnPoint, i + 1);
		_CharacterArray.push_back(hero);
		i++;
	}

	initTower();

	schedule(schedule_selector(GameLayer::updateViewPoint), 0.00f);
	scheduleOnce(schedule_selector(GameLayer::playGameOpeningAnimation), 0.5f);
}

Hero *GameLayer::addHero(CCString *character, CCString *role, CCString *group, CCPoint spawnPoint, int charNo)
{
	auto hero = Provider::create(character, role, group);
	if (hero->isPlayer())
	{
		currentPlayer = hero;
	}
	hero->setPosition(spawnPoint);
	hero->setSpawnPoint(spawnPoint);
	//NOTE: Set all characters speed to zero. (Control movement before game real start)
	hero->setWalkSpeed(0);
	if (is_same(group->getCString(), Akatsuki))
	{
		hero->_isFlipped = true;
		hero->setFlipX(true);
	}
	hero->setHPbar();
	hero->setShadows();
	hero->idle();
	hero->setCharNO(charNo);
	hero->schedule(schedule_selector(CharacterBase::setRestore2), 1.0f);

	addChild(hero, -hero->getPositionY());

	getGameModeHandler()->onCharacterInit(hero);
	return hero;
}

void GameLayer::playGameOpeningAnimation(float dt)
{
	getHudLayer()->playGameOpeningAnimation();

	setRand();
	auto path = random(2) == 0 ? "Audio/Menu/battle_start1.ogg" : "Audio/Menu/battle_start.ogg";
	SimpleAudioEngine::sharedEngine()->playEffect(path);

	scheduleOnce(schedule_selector(GameLayer::onGameStart), 0.75f);
}

void GameLayer::onGameStart(float dt)
{
	auto handler = getGameModeHandler();
	_isStarted = true;

	getHudLayer()->openingSprite->removeFromParent();
	getHudLayer()->openingSprite = nullptr;
	schedule(schedule_selector(GameLayer::updateGameTime), 1.0f);
	schedule(schedule_selector(GameLayer::checkBackgroundMusic), 2.0f);
	if (!handler->skipInitFlogs)
	{
		schedule(schedule_selector(GameLayer::addFlog), handler->flogSpawnDuration);
		initFlogs();
		addFlog(0);
	}

	setKeyEventHandler();

	for (auto hero : _CharacterArray)
	{
		// NOTE: Resume movement speed
		hero->setWalkSpeed(hero->_originSpeed);
		if (hero->isCom())
			hero->doAI();
	}

	getGameModeHandler()->onGameStart();
}

void GameLayer::initFlogs()
{
	addSprites("Element/hpBar/flogBar.plist");

	kName = FlogEnum::KotetsuFlog;
	aName = FlogEnum::FemalePainFlog;
}

void GameLayer::addFlog(float dt)
{
	CCString *KonohaFlogName = CCString::create(kName);
	CCString *AkatsukiFlogName = CCString::create(aName);

	int i;
	Flog *flog;
	float mainPosY;
	for (i = 0; i < NUM_FLOG; i++)
	{
		flog = Flog::create();
		flog->setID(KonohaFlogName, CCString::create(kRoleFlog), CCString::create(Konoha));
		if (i < NUM_FLOG / 2)
			mainPosY = (5.5 - i / 1.5) * 32;
		else
			mainPosY = (3.5 - i / 1.5) * 32;
		flog->_mainPosY = mainPosY;
		flog->setPosition(ccp(13 * 32, flog->_mainPosY));
		flog->setHPbar();
		flog->idle();
		flog->doAI();
		_KonohaFlogArray.push_back(flog);
		addChild(flog, -int(flog->getPositionY()));
	}

	for (i = 0; i < NUM_FLOG; i++)
	{
		flog = Flog::create();
		flog->setID(AkatsukiFlogName, CCString::create(kRoleFlog), CCString::create(Akatsuki));
		if (i < NUM_FLOG / 2)
			mainPosY = (5.5 - i / 1.5) * 32;
		else
			mainPosY = (3.5 - i / 1.5) * 32;
		flog->_mainPosY = mainPosY;
		flog->setPosition(ccp(83 * 32, flog->_mainPosY));
		flog->setHPbar();
		flog->idle();
		flog->doAI();
		_AkatsukiFlogArray.push_back(flog);
		addChild(flog, -flog->getPositionY());
	}
}

void GameLayer::initTower()
{
	addSprites(CCString::createWithFormat("Element/Tower/Tower%d.plist", mapId)->getCString());

	CCTMXObjectGroup *metaGroup = currentMap->objectGroupNamed("meta");
	CCArray *metaArray = metaGroup->getObjects();
	CCObject *pObject;
	int i = 0;

	CCARRAY_FOREACH(metaArray, pObject)
	{
		auto dict = (CCDictionary *)pObject;

		int metaX = ((CCString *)dict->objectForKey("x"))->intValue();
		int metaY = ((CCString *)dict->objectForKey("y"))->intValue();

		int metaWidth = ((CCString *)dict->objectForKey("width"))->intValue();
		int metaHeight = ((CCString *)dict->objectForKey("height"))->intValue();

		CCString *name = (CCString *)dict->objectForKey("name");

		Tower *tower = Tower::create();
		char towerName[7] = "abcdef";
		strncpy(towerName, name->getCString(), 6);
		if (is_same(towerName, Konoha))
		{
			tower->setID(name, CCString::create(kRoleTower), CCString::create(Konoha));
		}
		else
		{
			tower->setID(name, CCString::create(kRoleTower), CCString::create(Akatsuki));
			tower->setFlipX(true);
			tower->_isFlipped = true;
		}
		float posX = metaX + metaWidth / 2;
		float posY = metaY + metaHeight / 2;
		tower->setPosition(ccp(posX, posY));
		tower->setSpawnPoint(ccp(posX, posY));
		tower->setCharNO(i + 1);

		if (i == 1 || i == 4)
		{
			//4v4
			if (Cheats >= MaxCheats)
			{
				tower->setMaxHPValue(80000, false);
			}
			else
			{
				tower->setMaxHPValue(50000, false);
			}
			tower->setHPValue(tower->getMaxHPValue(), false);
		}
		tower->setHPbar();
		tower->_hpBar->setVisible(false);
		tower->idle();
		addChild(tower, -tower->getPositionY());

		_TowerArray.push_back(tower);
		i++;
	}
}

void GameLayer::initEffects()
{
	addSprites("Effects/SkillEffect.plist");
	skillEffectBatch = CCSpriteBatchNode::create("Effects/SkillEffect.png");
	addChild(skillEffectBatch, currentSkillTag);

	addSprites("Effects/DamageEffect.plist");
	damageEffectBatch = CCSpriteBatchNode::create("Effects/DamageEffect.png");
	addChild(damageEffectBatch, currentDamageTag);

	addSprites("Effects/Shadows.plist");
	shadowBatch = CCSpriteBatchNode::create("Effects/Shadows.png");
	addChild(shadowBatch, currentShadowTag);
}

void GameLayer::updateGameTime(float dt)
{
	_second += 1;
	if (_second == 60)
	{
		_minute += 1;
		_second = 0;
	}
	CCString *tempTime = CCString::createWithFormat("%02d:%02d", _minute, _second);
	_hudLayer->gameClock->setString(tempTime->getCString());

	int newValue = to_int(getTotalTM()->getCString()) + 1;
	setTotalTM(to_ccstring(newValue));
}

void GameLayer::updateViewPoint(float dt)
{
	if (!currentPlayer)
		return;
	CCPoint playerPoint;
	if (ougisChar)
	{
		playerPoint = ougisChar->getPosition();
	}
	else if (controlChar)
	{
		playerPoint = controlChar->getPosition();
	}
	else
	{
		playerPoint = currentPlayer->getPosition();
	}

	int x = MAX(playerPoint.x, winSize.width / 2);
	int y = MAX(playerPoint.y, winSize.width / 2);
	x = MIN(x, (currentMap->getMapSize().width * currentMap->getTileSize().width) - winSize.width / 2);
	y = MIN(y, (currentMap->getMapSize().height * currentMap->getTileSize().height) - winSize.height / 2);

	CCPoint actualPoint = ccp(x, y);
	CCPoint centerPoint = ccp(winSize.width / 2, y);
	CCPoint viewPoint = ccpSub(centerPoint, actualPoint);

	setPosition(viewPoint);
	// CCDirector::sharedDirector()->getScheduler()->setTimeScale(1.0f);
}

void GameLayer::setTowerState(int charNO)
{
	_hudLayer->setTowerState(charNO);
}

void GameLayer::updateHudSkillButtons()
{
	_hudLayer->updateSkillButtons();
}

void GameLayer::setHPLose(float percent)
{
	_hudLayer->setHPLose(percent);
}

void GameLayer::setCKRLose(bool isCRK2)
{
	_hudLayer->setCKRLose(isCRK2);
}

void GameLayer::setReport(const char *name1, const char *name2, CCString *killNum)
{
	_hudLayer->setReport(name1, name2, killNum);
}

void GameLayer::resetStatusBar()
{
	_hudLayer->status_hpbar->setRotation(0);
}

void GameLayer::setCoin(const char *value)
{
	_hudLayer->setCoin(value);
}

void GameLayer::removeOugisMark(int type)
{
	if (type == 1)
	{
		if (_hudLayer->skill4Button)
		{
			if (_hudLayer->skill4Button->lockLabel1)
			{
				_hudLayer->skill4Button->lockLabel1->removeFromParent();
				_hudLayer->skill4Button->lockLabel1 = nullptr;
			}
		}
	}
	else
	{
		if (_hudLayer->skill5Button)
		{
			if (_hudLayer->skill5Button->lockLabel1)
			{
				_hudLayer->skill5Button->lockLabel1->removeFromParent();
				_hudLayer->skill5Button->lockLabel1 = nullptr;
			}
		}
	}
}

void GameLayer::checkTower()
{
	int konohaTowerCount = 0;
	int akatsukiTowerCount = 0;

	for (auto tower : _TowerArray)
	{
		if (tower->isKonohaGroup())
			konohaTowerCount++;
		else
			akatsukiTowerCount++;
	}

	if (konohaTowerCount == 2)
	{
		aName = FlogEnum::PainFlog;
		kEXPBound = 50;
	}
	else if (konohaTowerCount == 1)
	{
		aName = FlogEnum::ObitoFlog;
		kEXPBound = 100;
	}

	if (akatsukiTowerCount == 2)
	{
		kName = FlogEnum::IzumoFlog;
		aEXPBound = 50;
	}
	else if (akatsukiTowerCount == 1)
	{
		kName = FlogEnum::KakashiFlog;
		aEXPBound = 100;
	}

	for (auto hero : getGameLayer()->_CharacterArray)
	{
		if (hero->isNotCom())
			continue;

		if (hero->isKonohaGroup())
		{
			hero->battleCondiction = konohaTowerCount - akatsukiTowerCount;
			if (konohaTowerCount == 1)
			{
				hero->isBaseDanger = true;
			}
		}
		else
		{
			hero->battleCondiction = akatsukiTowerCount - konohaTowerCount;
			if (_isHardCoreGame)
			{
				if (akatsukiTowerCount == 1)
				{
					hero->isBaseDanger = true;
				}
			}
		}
	}

	if (konohaTowerCount == 0 || akatsukiTowerCount == 0)
	{
		if (playerGroup == Konoha)
			onGameOver(konohaTowerCount != 0);
		else
			onGameOver(akatsukiTowerCount != 0);
	}
}

void GameLayer::clearDoubleClick()
{
	if (_hudLayer->skill1Button->getDoubleSkill() &&
		_hudLayer->skill1Button->_clickNum >= 1)
	{
		_hudLayer->skill1Button->setFreezeAction(nullptr);
		_hudLayer->skill1Button->beganAnimation();
	}
}

void GameLayer::JoyStickRelease()
{
	if (currentPlayer->getActionState() == State::WALK)
	{
		currentPlayer->idle();
	}
}

void GameLayer::JoyStickUpdate(CCPoint direction)
{
	if (!ougisChar)
	{
		//CCLOG("x:%f,y:%f",direction.x,direction.y);
		currentPlayer->walk(direction);
	}
}

void GameLayer::attackButtonClick(abType type)
{
	if (type == NAttack)
	{
		_isAttackButtonRelease = false;
	}

	if (type == Item1)
	{
		currentPlayer->setItem(type);
	}
	else
	{
		currentPlayer->attack(type);
	}
}

void GameLayer::gearButtonClick(gearType type)
{
	currentPlayer->useGear(type);
}

void GameLayer::attackButtonRelease()
{
	_isAttackButtonRelease = true;
}

void GameLayer::onPause()
{
	if (_isPause)
		return;

	_isPause = true;
	CCRenderTexture *snapshoot = CCRenderTexture::create(winSize.width, winSize.height);
	CCScene *f = CCDirector::sharedDirector()->getRunningScene();
	CCObject *pObject = f->getChildren()->objectAtIndex(0);
	BGLayer *bg = (BGLayer *)pObject;
	snapshoot->begin();
	bg->visit();

	visit();
	snapshoot->end();

	CCScene *pscene = CCScene::create();
	PauseLayer *layer = PauseLayer::create(snapshoot);
	pscene->addChild(layer);
	CCDirector::sharedDirector()->pushScene(pscene);
}

void GameLayer::onGear()
{
	if (!_enableGear)
		return;
	if (_isGear)
		return;
	_isGear = true;

	CCRenderTexture *snapshoot = CCRenderTexture::create(winSize.width, winSize.height);
	CCScene *f = CCDirector::sharedDirector()->getRunningScene();
	CCObject *pObject = f->getChildren()->objectAtIndex(0);
	BGLayer *bg = (BGLayer *)pObject;
	snapshoot->begin();
	bg->visit();

	visit();
	snapshoot->end();

	CCScene *pscene = CCScene::create();
	GearLayer *layer = GearLayer::create(snapshoot);
	_gearLayer = layer;
	layer->updatePlayerGear();
	pscene->addChild(layer);
	CCDirector::sharedDirector()->pushScene(pscene);
}

void GameLayer::onGameOver(bool isWin)
{
	removeKeyEventHandler();

	if (_isPause)
	{
		_isPause = false;
		CCDirector::sharedDirector()->popScene();
	}
	if (_isGear)
	{
		_isGear = false;
		CCDirector::sharedDirector()->popScene();
	}

	CCRenderTexture *snapshoot = CCRenderTexture::create(winSize.width, winSize.height);
	CCScene *f = CCDirector::sharedDirector()->getRunningScene();
	CCObject *pObject = f->getChildren()->objectAtIndex(0);
	BGLayer *bg = (BGLayer *)pObject;
	snapshoot->begin();
	bg->visit();
	visit();
	snapshoot->end();

	getGameModeHandler()->Internal_GameOver();

	CCScene *pscene = CCScene::create();
	GameOver *layer = GameOver::create(snapshoot);
	layer->setWin(isWin);
	pscene->addChild(layer);
	CCDirector::sharedDirector()->pushScene(pscene);
}

void GameLayer::onLeft()
{
	CCArray *childArray = getChildren();
	CCObject *pObject;

	CCARRAY_FOREACH(childArray, pObject)
	{
		auto ac = (CharacterBase *)pObject;
		ac->unscheduleUpdate();
		ac->unscheduleAllSelectors();
		CCNotificationCenter::sharedNotificationCenter()->removeAllObservers(ac);
	}

	LoadLayer::unloadAllCharsIMG(_CharacterArray);
	removeSprites(CCString::createWithFormat("Element/Tower/Tower%d.plist", mapId)->getCString());

	if (_isHardCoreGame)
	{
		removeSprites("Element/Roshi/Roshi.plist");
		removeSprites("Element/Han/Han.plist");
		KTools::prepareFileOGG(kGuardian_Roshi, true);
		KTools::prepareFileOGG(kGuardian_Han, true);
	}

	KTools::prepareFileOGG("Effect", true);
	KTools::prepareFileOGG("Ougis", true);

	_CharacterArray.clear();
	_TowerArray.clear();
	_KonohaFlogArray.clear();
	_AkatsukiFlogArray.clear();

	removeSprites("UI.plist");
	removeSprites("Map.plist");

	SimpleAudioEngine::sharedEngine()->end();

	lua_call_func("onGameOver");
}

void GameLayer::checkBackgroundMusic(float dt)
{
	if (CCUserDefault::sharedUserDefault()->getBoolForKey("isBGM"))
	{
		if (!SimpleAudioEngine::sharedEngine()->isBackgroundMusicPlaying())
		{
			if (!_isHardCoreGame)
			{
				SimpleAudioEngine::sharedEngine()->playBackgroundMusic(BATTLE_MUSIC);
			}
			else
			{
				int id = (mapId - 1) > 4 ? 4 : (mapId - 1);
				if (_playNum == 0)
				{
					SimpleAudioEngine::sharedEngine()->playBackgroundMusic(CCString::createWithFormat("Audio/Music/Battle%d.ogg", 2 + id * 3)->getCString(), false);
					_playNum++;
				}
				else if (_playNum == 1)
				{
					SimpleAudioEngine::sharedEngine()->playBackgroundMusic(CCString::createWithFormat("Audio/Music/Battle%d.ogg", 3 + id * 3)->getCString(), false);
					_playNum++;
				}
				else if (_playNum == 2)
				{
					SimpleAudioEngine::sharedEngine()->playBackgroundMusic(CCString::createWithFormat("Audio/Music/Battle%d.ogg", 1 + id * 3)->getCString(), false);
					_playNum = 0;
				}
			}
		}
	}
}

void GameLayer::setOugis(CCNode *sender)
{
	if (!_hudLayer->ougisLayer)
	{
		CCArray *childArray = getChildren();
		ougisChar = sender;
		auto Sender = (CharacterBase *)sender;
		CCObject *pObject;
		CCARRAY_FOREACH(childArray, pObject)
		{
			CCNode *object = (CCNode *)pObject;
			object->pauseSchedulerAndActions();
		}
		pauseSchedulerAndActions();

		updateViewPoint(0.01f);

		blend = CCLayerColor::create(ccc4(0, 0, 0, 200), winSize.width, winSize.height);
		blend->setPosition(ccp(-getPositionX(), 0));
		addChild(blend, 1000);
		sender->setZOrder(2000);

		if (CCUserDefault::sharedUserDefault()->getBoolForKey("isVoice"))
		{
			SimpleAudioEngine::sharedEngine()->playEffect(CCString::createWithFormat("Audio/Ougis/%s_ougis.ogg", Sender->getCharacter()->getCString())->getCString());
		}

		_hudLayer->setOugis(Sender->getCharacter(), Sender->getGroup());
	}
}

void GameLayer::removeOugis()
{
	ougisChar->setZOrder(-ougisChar->getPositionY());
	CCArray *childArray = getChildren();
	CCObject *pObject;
	CCARRAY_FOREACH(childArray, pObject)
	{
		CCNode *object = (CCNode *)pObject;
		object->resumeSchedulerAndActions();
	}
	resumeSchedulerAndActions();

	blend->removeFromParent();
	ougisChar = nullptr;
}

void GameLayer::setKeyEventHandler()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	CCDirector::sharedDirector()->getOpenGLView()->setAccelerometerKeyHook((cocos2d::CCEGLView::LPFN_ACCELEROMETER_KEYHOOK)(&GameLayer::LPFN_ACCELEROMETER_KEYHOOK));
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	glfwSetKeyCallback(_window, keyEventHandle);
#endif
}

void GameLayer::removeKeyEventHandler()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	CCDirector::sharedDirector()->getOpenGLView()->setAccelerometerKeyHook(nullptr);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	glfwSetKeyCallback(_window, nullptr);
#endif
}

int GameLayer::getMapCount()
{
	int index = 1;
	int mapCount = 0;
	auto fileUtils = CCFileUtils::sharedFileUtils();
	while (fileUtils->isFileExist(CCString::createWithFormat("Tiles/%d.tmx", index++)->getCString()))
		mapCount++;
	CCLOG("===== Found %d maps =====", mapCount);
	return mapCount;
}

void GameLayer::invokeAllCallbacks()
{
	isHUDInitialized = true;
	if (callbackssList.size() > 0)
	{
		for (auto &callback : callbackssList)
			callback();
		callbackssList.clear();
	}
}

CCPoint GameLayer::getCustomSpawnPoint(HeroData &data)
{
	data.isInit = true;
	return data.group == Konoha ? ccp(432, 80) : ccp(2608, 80);
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#define isPressed(__KEY__) _isPressed(__KEY__)
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
#include "glfw3.h"
#define isPressed(__KEY__) glfwGetKey(_window, __KEY__)
#endif

/**
 * Use __W __S __D __A control when to force move
 */
#define MOVE(__W, __S, __D, __A, name, keyState)                                    \
	{                                                                               \
		if (keyState)                                                               \
			_gLayer->_lastPressedMovementKey = name;                                \
		else if (_gLayer->_lastPressedMovementKey == name)                          \
			_gLayer->_lastPressedMovementKey = -100;                                \
		int horizontal;                                                             \
		int vertical;                                                               \
		if (__W)                                                                    \
		{                                                                           \
			vertical = (isPressed(KEY_W) ? 1 : -1);                                 \
		}                                                                           \
		else if (__S)                                                               \
		{                                                                           \
			vertical = (isPressed(KEY_S) ? -1 : 1);                                 \
		}                                                                           \
		else                                                                        \
		{                                                                           \
			vertical = (isPressed(KEY_W) ? 1 : -1) + (isPressed(KEY_S) ? -1 : 1);   \
			vertical = abs(vertical) > 1 ? vertical / 2 : vertical;                 \
		}                                                                           \
		if (__D)                                                                    \
		{                                                                           \
			horizontal = (isPressed(KEY_D) ? 1 : -1);                               \
		}                                                                           \
		else if (__A)                                                               \
		{                                                                           \
			horizontal = (isPressed(KEY_A) ? -1 : 1);                               \
		}                                                                           \
		else                                                                        \
		{                                                                           \
			horizontal = (isPressed(KEY_D) ? 1 : -1) + (isPressed(KEY_A) ? -1 : 1); \
			horizontal = abs(horizontal) > 1 ? horizontal / 2 : horizontal;         \
		}                                                                           \
		if (horizontal != 0 || vertical != 0)                                       \
		{                                                                           \
			if (!_gLayer->ougisChar)                                                \
				_gLayer->currentPlayer->walk(ccp(horizontal, vertical));            \
		}                                                                           \
		else if (_gLayer->currentPlayer->getActionState() == State::WALK)           \
		{                                                                           \
			_gLayer->_lastPressedMovementKey = -100;                                \
			_gLayer->currentPlayer->idle();                                         \
		}                                                                           \
		break;                                                                      \
	}

#define ON_GEAR_BY(__ID__, __KEY_STATE__)                                  \
	if (_gLayer->_isGear && __KEY_STATE__)                                 \
	{                                                                      \
		auto gearArray = _gLayer->_gearLayer->_screwLayer->getGearArray(); \
		if (gearArray.size() >= __ID__ - 1)                                \
		{                                                                  \
			auto gear_btn = gearArray.at(__ID__ - 1);                      \
			if (gear_btn)                                                  \
				gear_btn->click();                                         \
		}                                                                  \
	}                                                                      \
	break;

bool GameLayer::checkHasAnyMovement()
{
	if (_gLayer)
	{
		if (_gLayer->_lastPressedMovementKey != -100)
		{
			keyEventHandle(_window, _gLayer->_lastPressedMovementKey, 0, 1, 0);
			return true;
		}
	}
	return false;
}

/** NOTE: Impl key listener */
void GameLayer::keyEventHandle(GLFWwindow *window, int key, int scancode, int keyState, int mods)
{
	//NOTE: only attack button can hold
	// Other keys is only click
	if (keyState == 2 && key != KEY_J)
		return;
	switch (key)
	{
	case KEY_W:
		// case KEY_UP:
		MOVE(keyState, 0, 0, 0, KEY_W, keyState);
	case KEY_S:
		// case KEY_DOWN:
		MOVE(0, keyState, 0, 0, KEY_S, keyState);
	case KEY_A:
		// case KEY_LEFT:
		MOVE(0, 0, keyState, 0, KEY_A, keyState);
	case KEY_D:
		// case KEY_RIGHT:
		MOVE(0, 0, 0, keyState, KEY_D, keyState);
	case KEY_J:
		if (keyState)
			_gLayer->_hudLayer->nAttackButton->click();
		else
			_gLayer->_isAttackButtonRelease = true;
		break;
	case KEY_L: // Ramen button
		if (keyState)
			_gLayer->_hudLayer->item1Button->click();
		break;
	case KEY_H: // Ougis 2 buttons
		if (keyState)
			_gLayer->_hudLayer->skill5Button->click();
		break;
	case KEY_K: // Ougis 1 buttons
		if (keyState)
			_gLayer->_hudLayer->skill4Button->click();
		break;
	case KEY_U: // skill 1
		if (keyState)
			_gLayer->_hudLayer->skill1Button->click();
		break;
	case KEY_I: // skill 2
		if (keyState)
			_gLayer->_hudLayer->skill2Button->click();
		break;
	case KEY_O: // skill 3
		if (keyState)
			_gLayer->_hudLayer->skill3Button->click();
		break;
		// Gear buttons
	case KEY_1:
	case KEY_KP_1:
		ON_GEAR_BY(1, keyState);
	case KEY_2:
	case KEY_KP_2:
		ON_GEAR_BY(2, keyState);
	case KEY_3:
	case KEY_KP_3:
		ON_GEAR_BY(3, keyState);
	case KEY_4:
	case KEY_KP_4:
		ON_GEAR_BY(4, keyState);
	case KEY_5:
	case KEY_KP_5:
		ON_GEAR_BY(5, keyState);
	case KEY_6:
	case KEY_KP_6:
		ON_GEAR_BY(6, keyState);
	case KEY_7:
	case KEY_KP_7:
		ON_GEAR_BY(7, keyState);
	case KEY_8:
	case KEY_KP_8:
		ON_GEAR_BY(8, keyState);
	case KEY_9:
	case KEY_KP_9:
		ON_GEAR_BY(9, keyState);
	case KEY_0:
	case KEY_KP_0:
		break;
	/* Item buttons */
	// Item 1 & Purchase
	case KEY_B:
		if (keyState)
		{
			if (_gLayer->_isGear)
				_gLayer->_gearLayer->confirmPurchase();
			else
				_gLayer->_hudLayer->getItem3Button()->click();
		}
		break;
	// Item 2
	case KEY_N:
		if (keyState)
			_gLayer->_hudLayer->getItem4Button()->click();
		break;
	// Item 3
	case KEY_M:
		if (keyState)
			_gLayer->_hudLayer->getItem2Button()->click();
		break;
	case KEY_ESCAPE:
	case KEY_ENTER:
		if (keyState && _gLayer->_isStarted == true)
		{
			if (_gLayer->_isPause)
			{
				CCDirector::sharedDirector()->popScene();
				_gLayer->_isPause = false;
			}
			else if (_gLayer->_isGear)
			{
				CCDirector::sharedDirector()->popScene();
				_gLayer->_isGear = false;
			}
			else
			{
				_gLayer->onPause(); // enter pause menu
			}
		}
		break;
	case KEY_SPACE:
		if (_gLayer->_enableGear && _gLayer->_isStarted && keyState && !_gLayer->_isPause && !_gLayer->_isGear)
			_gLayer->onGear(); // enter gear shop
		break;
	case KEY_F11:
		// if (keyState)
		// {
		// 	// SET_FULL_SCREEN_MODE(_window, _isFullScreen);
		// 	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		// 	if (nullptr == monitor)
		// 	{
		// 		return;
		// 	}
		// 	const GLFWvidmode *videoMode = glfwGetVideoMode(monitor);
		// 	int width, height;
		// 	glfwGetWindowSize(_window, &width, &height);
		// 	if (_isFullScreen)
		// 	{
		// 		glfwSetWindowMonitor(_window, nullptr, videoMode->width / 2, videoMode->height / 2, WIDTH, HEIGHT, videoMode->refreshRate);
		// 		glfwSetWindowPos(_window,
		// 						 (videoMode->width - WIDTH) / 2,
		// 						 (videoMode->height - HEIGHT) * 0.35f);
		// 	}
		// 	else
		// 	{
		// 		glfwSetWindowMonitor(_window, nullptr, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
		// 		CCDirector::sharedDirector()->getOpenGLView()->updateFrameSize(videoMode->width,videoMode->height);
		// 	}
		// 	_isFullScreen = !_isFullScreen;
		// }
		break;
	}
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

void GameLayer::LPFN_ACCELEROMETER_KEYHOOK(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		keyEventHandle(nullptr, wParam, 0, 1, 0);
		break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		keyEventHandle(nullptr, wParam, 0, 0, 0);
		break;
	}
}

#endif

#endif
