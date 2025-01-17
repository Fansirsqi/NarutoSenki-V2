#include "GameOver.h"
#include "GameLayer.h"
#include "Core/Hero.hpp"
#include "GameMode/GameModeImpl.h"

GameOver::GameOver()
{
	exitLayer = nullptr;
	cheatLayer = nullptr;
	isPosting = false;
	refreshBtn = nullptr;
}

GameOver::~GameOver()
{
	CCAnimationCache::purgeSharedAnimationCache();
	CCSpriteFrameCache::sharedSpriteFrameCache()->removeUnusedSpriteFrames();
	CCTextureCache::sharedTextureCache()->removeUnusedTextures();
}

bool GameOver::init(CCRenderTexture *snapshoot)
{
	RETURN_FALSE_IF(!CCLayer::init());

	SimpleAudioEngine::sharedEngine()->stopAllEffects();
	SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(true);

	CCTexture2D *bgTexture = snapshoot->getSprite()->getTexture();
	CCSprite *bg = CCSprite::createWithTexture(bgTexture);
	bg->setAnchorPoint(ccp(0, 0));
	bg->setFlipY(true);
	addChild(bg, 0);

	CCLayer *blend = CCLayerColor::create(ccc4(0, 0, 0, 150), winSize.width, winSize.height);
	addChild(blend, 1);

	//produce the menu_bar
	CCSprite *menu_bar_b = CCSprite::create("menu_bar2.png");
	menu_bar_b->setAnchorPoint(ccp(0, 0));
	FULL_SCREEN_SPRITE(menu_bar_b);
	addChild(menu_bar_b, 2);

	CCSprite *menu_bar_t = CCSprite::create("menu_bar3.png");
	menu_bar_t->setAnchorPoint(ccp(0, 0));
	menu_bar_t->setPosition(ccp(0, winSize.height - menu_bar_t->getContentSize().height));
	FULL_SCREEN_SPRITE(menu_bar_t);
	addChild(menu_bar_t, 2);

	CCSprite *result_title = CCSprite::createWithSpriteFrameName("result_title.png");
	result_title->setAnchorPoint(ccp(0, 0));
	result_title->setPosition(ccp(2, winSize.height - result_title->getContentSize().height - 2));
	addChild(result_title, 3);

	result_bg = CCSprite::createWithSpriteFrameName("gameover_bg.png");
	result_bg->setAnchorPoint(ccp(0.5f, 0.5f));
	result_bg->setScale(0.5f);
	result_bg->setPosition(ccp(winSize.width / 2, winSize.height / 2 - 6));
	addChild(result_bg, 4);

	auto su = CCScaleTo::create(0.2f, 1.0);
	auto seq = CCSequence::create(su, CCCallFunc::create(this, callfunc_selector(GameOver::listResult)), nullptr);
	result_bg->runAction(seq);

	return true;
}

void GameOver::listResult()
{
	if (getGameLayer()->_isHardCoreGame)
		SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/battle_over1.ogg");

	else
		SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/battle_over.ogg");

	auto path = CCString::createWithFormat("%s_half.png", getGameLayer()->currentPlayer->getCharacter()->getCString());
	CCSprite *half = CCSprite::createWithSpriteFrameName(path->getCString());

	if (getGameLayer()->currentPlayer->isCharacter("Konan",
												   "Karin",
												   "Suigetsu",
												   "Hidan",
												   "Tobirama",
												   "Tsunade",
												   "Kankuro",
												   "SageJiraiya",
												   "Minato",
												   "Tobi") ||
		getGameLayer()->currentPlayer->isCharacter("Lee",
												   "RockLee",
												   "Hinata",
												   "Asuma",
												   "Chiyo",
												   "Kisame"))
	{
		half->setFlipX(true);
	}

	half->setAnchorPoint(ccp(0, 0));
	half->setPosition(ccp(winSize.width / 2 + result_bg->getContentSize().width / 2 - half->getContentSize().width, result_bg->getPositionY() - result_bg->getContentSize().height / 2 + 9));
	addChild(half, 5);

	CCSprite *list_bg = CCSprite::createWithSpriteFrameName("list_bg.png");
	list_bg->setAnchorPoint(ccp(0, 0));
	list_bg->setPosition(ccp(winSize.width / 2 - result_bg->getContentSize().width / 2 + 2, result_bg->getPositionY() - result_bg->getContentSize().height / 2 + 26));
	addChild(list_bg, 5);

	int _hour = getGameLayer()->_minute / 60;
	int _minute = getGameLayer()->_minute % 60;

	CCSprite *timeBG = CCSprite::createWithSpriteFrameName("time_bg.png");
	timeBG->setAnchorPoint(ccp(0, 0));
	timeBG->setPosition(ccp(winSize.width / 2 + result_bg->getContentSize().width / 2 - 11 - timeBG->getContentSize().width, result_bg->getPositionY() - result_bg->getContentSize().height / 2 + 46));
	addChild(timeBG, 6);

	auto tempTime = CCString::createWithFormat("%02d:%02d:%02d", _hour, _minute, getGameLayer()->_second)->getCString();
	auto gameClock = CCLabelBMFont::create(tempTime, "Fonts/1.fnt");
	gameClock->setAnchorPoint(ccp(0.5f, 0));
	gameClock->setPosition(ccp(timeBG->getPositionX() + timeBG->getContentSize().width / 2, timeBG->getPositionY() + 3));
	gameClock->setScale(0.48f);
	addChild(gameClock, 7);

	float _totalSecond = getGameLayer()->_minute * 60 + getGameLayer()->_second;
	float resultScore = 0;
	int killDead = to_int(getGameLayer()->currentPlayer->getKillNum()->getCString()) - getGameLayer()->currentPlayer->_deadNum;

	if (_totalSecond != to_int(getGameLayer()->getTotalTM()->getCString()))
	{
		SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(true);
		CCDirector::sharedDirector()->end();
		return;
	}

	if (getGameLayer()->_isHardCoreGame)
	{
		if (_totalSecond > 900.0)
			resultScore = float((killDead / float(_totalSecond / 60)) / 3) * 100;
		else
			resultScore = float((killDead - ((_totalSecond / 60 - 15) * 3)) / 45) * 100;
	}
	else
	{
		if (_totalSecond > 600.0)
			resultScore = float((killDead / float(_totalSecond / 60)) / 4) * 100;
		else
			resultScore = float((killDead - ((_totalSecond / 60 - 10) * 4)) / 40) * 100;
	}

	if (_totalSecond < 2 * 30 + 5 && _isWin)
	{
		SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(true);
		CCDirector::sharedDirector()->end();
		return;
	}

	int i = 0;
	int konohaKill = 0;
	int akatsukiKill = 0;

	for (auto hero : getGameLayer()->_CharacterArray)
	{
		if (hero->isClone() ||
			hero->isSummon() ||
			hero->isKugutsu() ||
			hero->isGuardian())
		{
			continue;
		}

		//4v4
		if (Cheats >= MaxCheats)
		{
			if (hero->isNotPlayer())
			{
				continue;
			}
		}

		if (hero->_isControlled)
		{
			hero->_isControlled = false;
			hero->changeGroup();
		}

		auto path = CCString::createWithFormat(("%s_small.png"), hero->getCharacter()->getCString());
		CCSprite *avator_small = CCSprite::createWithSpriteFrameName(path->getCString());
		avator_small->setAnchorPoint(ccp(0, 0));

		int realKillNum = to_int(hero->getKillNum()->getCString());

		if (hero->isKonohaGroup())
		{
			konohaKill += realKillNum;
		}
		else
		{
			akatsukiKill += realKillNum;
		}

		auto killNum = CCLabelBMFont::create(to_cstr(realKillNum), "Fonts/1.fnt");
		killNum->setScale(0.3f);

		auto deadNum = CCLabelBMFont::create(to_cstr(hero->_deadNum), "Fonts/1.fnt");
		deadNum->setScale(0.3f);

		auto flogNum = CCLabelBMFont::create(to_cstr(hero->_flogNum), "Fonts/1.fnt");
		flogNum->setScale(0.3f);

		float posX = winSize.width / 2 - result_bg->getContentSize().width / 2 + 2;

		if (hero->isKonohaGroup())
		{
			avator_small->setPosition(ccp(posX, result_bg->getPositionY() - result_bg->getContentSize().height / 2 + 152 - (i * 25)));
			killNum->setPosition(ccp(posX + 44, avator_small->getPositionY() + avator_small->getContentSize().height / 2));
			deadNum->setPosition(ccp(posX + 68, avator_small->getPositionY() + avator_small->getContentSize().height / 2));
			flogNum->setPosition(ccp(posX + 94, avator_small->getPositionY() + avator_small->getContentSize().height / 2));
		}
		else
		{
			avator_small->setPosition(ccp(posX, result_bg->getPositionY() - result_bg->getContentSize().height / 2 + 76 - ((i - 3) * 25)));
			killNum->setPosition(ccp(posX + 44, avator_small->getPositionY() + avator_small->getContentSize().height / 2));
			deadNum->setPosition(ccp(posX + 68, avator_small->getPositionY() + avator_small->getContentSize().height / 2));
			flogNum->setPosition(ccp(posX + 94, avator_small->getPositionY() + avator_small->getContentSize().height / 2));
		}

		if (hero->getGearArray() && hero->getGearArray()->count() > 0)
		{
			CCObject *pObject;
			int i = 0;
			CCARRAY_FOREACH(hero->getGearArray(), pObject)
			{
				CCString *tmpGear = (CCString *)pObject;
				CCSprite *gearIcon = CCSprite::createWithSpriteFrameName(CCString::createWithFormat("gear_%02d.png", to_int(tmpGear->getCString()))->getCString());
				gearIcon->setPosition(ccp(flogNum->getPositionX() + 22 + i * 19, flogNum->getPositionY() - 1));
				gearIcon->setScale(0.5f);
				addChild(gearIcon, 7);
				i++;
			}
		}

		addChild(avator_small, 7);
		addChild(killNum, 7);
		addChild(deadNum, 7);
		addChild(flogNum, 7);

		i++;
	}

	if (_totalSecond > 900 && getGameLayer()->_isSurrender)
	{
		if (is_same(getGameLayer()->currentPlayer->getGroup()->getCString(), Konoha))
		{
			if (konohaKill > akatsukiKill)
				_isWin = true;
		}
		else
		{
			if (akatsukiKill > konohaKill)
				_isWin = true;
		}
	}

	if (Cheats < MaxCheats)
	{
		if (akatsukiKill + konohaKill != to_int(getGameLayer()->getTotalKills()->getCString()))
		{
			SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(true);
			CCDirector::sharedDirector()->end();
			return;
		}

		int realKillNum = to_int(getGameLayer()->currentPlayer->getKillNum()->getCString());

		string tempReward = getGameLayer()->_isHardCoreGame ? "FDDD" : "ONNN";
		KTools::decode(tempReward);

		int coins = KTools::readCoinFromSQL();
		int rewardNum;
		if (_isWin)
			rewardNum = realKillNum * 75 + to_int(tempReward.c_str());
		else
			rewardNum = realKillNum * 50;

		CCSprite *coinBG = CCSprite::createWithSpriteFrameName("coin_bg.png");
		coinBG->setAnchorPoint(ccp(0, 0));
		coinBG->setPosition(ccp(winSize.width / 2 + result_bg->getContentSize().width / 2 - coinBG->getContentSize().width - 11, result_bg->getPositionY() - result_bg->getContentSize().height / 2 + 12));
		addChild(coinBG, 6);

		CCSprite *adExtra = CCSprite::createWithSpriteFrameName("adExtra.png");
		adExtra->setAnchorPoint(ccp(0.5f, 0));
		adExtra->setPosition(ccp(coinBG->getPositionX() + 70, coinBG->getPositionY() + 22));
		addChild(adExtra, 7);

		const char *extraCoin;
		int tempCoin;
		if (_isWin)
		{
			string tempEtra = "idd4";
			KTools::decode(tempEtra);
			tempCoin = rewardNum + coins + to_int(tempEtra.c_str());
			extraCoin = "+500";
		}
		else
		{
			tempCoin = rewardNum + coins;
			extraCoin = "+0";
		}

		KTools::saveToSQLite("GameRecord", "coin", std::to_string(tempCoin).c_str(), false);

		auto extraLabel = CCLabelBMFont::create(extraCoin, "Fonts/yellow.fnt");
		extraLabel->setScale(0.5f);
		extraLabel->setAnchorPoint(ccp(0.5f, 0));
		extraLabel->setPosition(ccp(coinBG->getPositionX() + 68, coinBG->getPositionY() + 3));
		addChild(extraLabel, 7);

		auto rewardCoin = to_cstr(rewardNum);
		auto rewardLabel = CCLabelBMFont::create(rewardCoin, "Fonts/yellow.fnt");
		rewardLabel->setAnchorPoint(ccp(0.5f, 0));
		rewardLabel->setPosition(ccp(coinBG->getPositionX() + 28, coinBG->getPositionY() + 3));
		rewardLabel->setScale(0.55f);
		addChild(rewardLabel, 7);
	}

	const char *imgSrc;

	if (_isWin)
	{
		if (resultScore >= 140 && getGameLayer()->_isHardCoreGame && getGameLayer()->_isRandomChar && !getGameLayer()->_enableGear)
			imgSrc = "result_SSSR.png";
		else if (resultScore >= 120 && getGameLayer()->_isHardCoreGame && getGameLayer()->_isRandomChar && !getGameLayer()->_enableGear)
			imgSrc = "result_SSR.png";
		else if (resultScore >= 100 && getGameLayer()->_isHardCoreGame && getGameLayer()->_isRandomChar && !getGameLayer()->_enableGear)
			imgSrc = "result_SR.png";
		else if (resultScore >= 140)
			imgSrc = "result_SSS.png";
		else if (resultScore >= 120)
			imgSrc = "result_SS.png";
		else if (resultScore >= 100)
			imgSrc = "result_S.png";
		else if (resultScore >= 80)
			imgSrc = "result_A.png";
		else if (resultScore >= 60)
			imgSrc = "result_B.png";
		else
			imgSrc = "result_C.png";
	}
	else if (!_isWin)
	{
		imgSrc = "result_Defeat.png";
	}

	if (imgSrc)
	{
		CCSprite *recordSprite = CCSprite::createWithSpriteFrameName(imgSrc);
		recordSprite->setAnchorPoint(ccp(0, 0));
		recordSprite->setPosition(ccp(winSize.width / 2 + result_bg->getContentSize().width / 2 - recordSprite->getContentSize().width - 12, result_bg->getPositionY() - result_bg->getContentSize().height / 2 + 88));
		addChild(recordSprite, 7);

		if (_isWin && getGameLayer()->_isHardCoreGame)
		{
			finnalScore = resultScore + float(getGameLayer()->currentPlayer->_flogNum) / 100;
			// CCString *recordString = CCString::createWithFormat("%0.2fPts", finnalScore);

			// auto recordScore = CCLabelBMFont::create(recordString->getCString(), "Fonts/1.fnt");
			// recordScore->setAnchorPoint(ccp(1, 0.5f));
			// recordScore->setPosition(ccp(recordSprite->getPositionX() + recordSprite->getContentSize().width, recordSprite->getPositionY() - 7));
			// recordScore->setScale(0.35f);
			// addChild(recordScore, 10);

			// upload_btn = CCMenuItemSprite::create(CCSprite::createWithSpriteFrameName("upload_btn.png"), CCSprite::createWithSpriteFrameName("upload_btn.png"), nullptr, this, menu_selector(GameOver::onUPloadBtn));
			// CCMenu *upMenu = CCMenu::create(upload_btn, nullptr);
			// upload_btn->setAnchorPoint(ccp(1.0, 0));
			// upMenu->setPosition(ccp(winSize.width / 2 + result_bg->getContentSize().width / 2 - 14, winSize.height / 2 + result_bg->getContentSize().height / 2 - 62));
			// addChild(upMenu, 7);

			// detailRecord = CCString::createWithFormat("%02d:%02d,%s,%d,%d", _minute, getGameLayer()->_second, getGameLayer()->currentPlayer->getKillNum()->getCString(), getGameLayer()->currentPlayer->_deadNum, getGameLayer()->currentPlayer->_flogNum)->getCString();
		}
		if (Cheats < MaxCheats)
		{
			resultChar = getGameLayer()->currentPlayer->getCharacter()->getCString();
			if (getGameLayer()->currentPlayer->isCharacter("SageNaruto"))
				resultChar = "Naruto";
			else if (getGameLayer()->currentPlayer->isCharacter("RikudoNaruto"))
				resultChar = "Naruto";
			else if (getGameLayer()->currentPlayer->isCharacter("SageJiraiya"))
				resultChar = "Jiraiya";
			else if (getGameLayer()->currentPlayer->isCharacter("ImmortalSasuke"))
				resultChar = "Sasuke";
			else if (getGameLayer()->currentPlayer->isCharacter("RockLee"))
				resultChar = "Lee";

			if (_isWin)
			{
				int winNum = KTools::readWinNumFromSQL(resultChar);
				if (resultScore >= 140)
					winNum += 3;
				else if (resultScore >= 120)
					winNum += 2;
				else
					winNum += 1;

				auto realWin = std::to_string(winNum).c_str();
				KTools::saveSQLite("CharRecord", "name", resultChar, "column1", (char *)realWin, false);

				if (getGameLayer()->_isRandomChar && resultScore >= 120)
				{
					if (getGameLayer()->currentPlayer->_isControlled)
					{
						getGameLayer()->currentPlayer->_isControlled = false;
						getGameLayer()->currentPlayer->changeGroup();
					}

					for (auto hero : getGameLayer()->_CharacterArray)
					{
						if (hero->isClone() ||
							hero->isPlayer() ||
							hero->isSummon() ||
							hero->isKugutsu() ||
							hero->isGuardian())
						{
							continue;
						}

						if (hero->_isControlled)
						{
							hero->_isControlled = false;
							hero->changeGroup();
						}

						if (is_same(hero->getGroup()->getCString(), getGameLayer()->currentPlayer->getGroup()->getCString()))
						{
							int winNum = KTools::readWinNumFromSQL(hero->getCharacter()->getCString());
							if (resultScore >= 140)
								winNum += 2;
							else
								winNum += 1;

							auto realWin = std::to_string(winNum).c_str();
							KTools::saveSQLite("CharRecord", "name", hero->getCharacter()->getCString(), "column1", (char *)realWin, false);
						}
					}
				}

				auto bestTime = KTools::readSQLite("CharRecord", "name", resultChar, "column3");
				if (bestTime.empty())
				{
					KTools::saveSQLite("CharRecord", "name", resultChar, "column3", (char *)tempTime, false);
				}
				else
				{
					int recordHour = to_int(bestTime.substr(0, 2).c_str());
					int recordMinute = to_int(bestTime.substr(3, 2).c_str());
					int recordSecond = to_int(bestTime.substr(6, 2).c_str());

					int recordTime = recordHour * 60 * 60 + recordMinute * 60 + recordSecond;
					int currentTime = getGameLayer()->_minute * 60 + getGameLayer()->_second;
					bool isNewRecord = currentTime < recordTime;

					if (isNewRecord)
					{
						KTools::saveSQLite("CharRecord", "name", resultChar, "column3", (char *)tempTime, false);
					}
				}
			}
		}
	}

	// auto version = CCLabelBMFont::create(Cheats < MaxCheats ? GAMEOVER_VER : GAMEOVER_VER, "Fonts/1.fnt");
	auto version = CCLabelBMFont::create(GAMEOVER_VER, "Fonts/1.fnt");
	version->setPosition(ccp(winSize.width / 2 + 94, result_bg->getPositionY() - result_bg->getContentSize().height / 2 + 6));
	version->setScale(0.3f);
	addChild(version, 5);

	CCMenuItem *btm_btn = CCMenuItemSprite::create(CCSprite::createWithSpriteFrameName("close_btn1.png"), CCSprite::createWithSpriteFrameName("close_btn2.png"), nullptr, this, menu_selector(GameOver::onBackToMenu));
	CCMenu *overMenu = CCMenu::create(btm_btn, nullptr);
	overMenu->setPosition(ccp(winSize.width / 2 + result_bg->getContentSize().width / 2 - 12, winSize.height / 2 + result_bg->getContentSize().height / 2 - 18));
	addChild(overMenu, 7);

	getGameLayer()->_isSurrender = false;

	// Reset cheats value
	auto mode = getGameMode();
	if (mode == GameMode::FourVsFour || mode == GameMode::HardCore_4Vs4)
	{
		Cheats = getGameModeHandler()->getOldCheats();
	}
}

void GameOver::onUPloadBtn(CCObject *sender)
{
	CCTips *tip = CCTips::create("ServerMainten");
	addChild(tip, 5000);

	return;
}

void GameOver::onBackToMenu(CCObject *sender)
{
	if (!exitLayer)
	{
		SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/select.ogg");
		exitLayer = CCLayer::create();

		CCSprite *exit_bg = CCSprite::createWithSpriteFrameName("confirm_bg.png");
		exit_bg->setPosition(ccp(winSize.width / 2, winSize.height / 2));

		CCSprite *comfirm_title = CCSprite::createWithSpriteFrameName("confirm_title.png");
		comfirm_title->setPosition(ccp(winSize.width / 2, winSize.height / 2 + 38));

		CCSprite *btm_text = CCSprite::createWithSpriteFrameName("btm_text.png");
		btm_text->setPosition(ccp(winSize.width / 2, winSize.height / 2 + 8));

		CCMenuItem *yes_btn = CCMenuItemSprite::create(CCSprite::createWithSpriteFrameName("yes_btn1.png"), CCSprite::createWithSpriteFrameName("yes_btn2.png"), this, menu_selector(GameOver::onLeft));
		CCMenuItem *no_btn = CCMenuItemSprite::create(CCSprite::createWithSpriteFrameName("no_btn1.png"), CCSprite::createWithSpriteFrameName("no_btn2.png"), this, menu_selector(GameOver::onCancel));

		CCMenu *confirm_menu = CCMenu::create(yes_btn, no_btn, nullptr);
		confirm_menu->alignItemsHorizontallyWithPadding(24);
		confirm_menu->setPosition(ccp(winSize.width / 2, winSize.height / 2 - 30));

		exitLayer->addChild(exit_bg, 1);
		exitLayer->addChild(confirm_menu, 2);
		exitLayer->addChild(comfirm_title, 2);
		exitLayer->addChild(btm_text, 2);
		addChild(exitLayer, 500);
	}
}

void GameOver::onLeft(CCObject *sender)
{
	SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/confirm.ogg");

	getGameLayer()->_isExiting = true;
	CCDirector::sharedDirector()->popScene();
}

void GameOver::onCancel(CCObject *sender)
{
	SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/cancel.ogg");
	exitLayer->removeFromParent();
	exitLayer = nullptr;
}

GameOver *GameOver::create(CCRenderTexture *snapshoot)
{
	GameOver *pl = new GameOver();
	if (pl && pl->init(snapshoot))
	{
		pl->autorelease();
		return pl;
	}
	else
	{
		delete pl;
		return nullptr;
	}
}
