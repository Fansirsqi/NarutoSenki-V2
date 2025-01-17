#pragma once
#include "Core/Provider.hpp"
#include "GameMode/IGameModeHandler.hpp"

class ModeRandomDeathmatch : public IGameModeHandler
{
public:
	const uint8_t kRebornCount = 40;
	const uint8_t kHeroAmount = 3;

private:
	uint8_t konohaRebornCount;
	uint8_t akatsukiRebornCount;
	uint8_t konohaLiveCount;
	uint8_t akatsukiLiveCount;

public:
	void init()
	{
		CCLOG("Enter Random Deathmatch mode.");

		konohaRebornCount = kRebornCount;
		akatsukiRebornCount = kRebornCount;

		konohaLiveCount = kHeroAmount;
		akatsukiLiveCount = kHeroAmount;

		gd.isHardCore = true;
	}

	void onInitHeros()
	{
		initHeros(kHeroAmount, kHeroAmount);
	}

	void onGameStart()
	{
	}

	void onGameOver()
	{
	}

	void onCharacterInit(CharacterBase *c)
	{
	}

	void onCharacterDead(CharacterBase *c)
	{
		CCLOG("[Character Dead] Name: %s, Role: %s, Group: %s", c->getCharacter()->getCString(), c->getRole()->getCString(), c->getGroup()->getCString());

		if (c->isPlayerOrCom())
		{
			uint8_t &count = c->isKonohaGroup() ? konohaRebornCount : akatsukiRebornCount;
			uint8_t &liveCount = c->isKonohaGroup() ? konohaLiveCount : akatsukiLiveCount;
			if (count == 0)
			{
				c->enableReborn = false;
				if (liveCount == 0)
					getGameLayer()->onGameOver(!c->isGroup(playerGroup));
			}
			else
			{
				count--;
				liveCount--;
				setRandomHero(c);
				c->unschedule(schedule_selector(CharacterBase::resumeAction));
			}
		}
	}

	void onCharacterReborn(CharacterBase *c)
	{
		CCLOG("[Character Reborn] Name: %s, Role: %s, Group: %s", c->getCharacter()->getCString(), c->getRole()->getCString(), c->getGroup()->getCString());

		if (c->isPlayerOrCom())
		{
			uint8_t liveCount = c->isKonohaGroup() ? konohaLiveCount++ : akatsukiLiveCount++;
		}

		if (c->changeCharId > -1)
		{
			auto gameLayer = getGameLayer();
			// initial a new random character
			auto newCharName = heroVector.at(c->changeCharId);
			CCLOG("[Change Character] %s from %s to %s", c->getRole()->getCString(), c->getCharacter()->getCString(), newCharName.c_str());

			if (c->isCharacter(newCharName))
			{
				c->changeCharId = -1;
				return;
			}
			else
			{
				// If the character has changed, then cleanup
				CCNotificationCenter::sharedNotificationCenter()->removeAllObservers(c);
				if (c->_shadow)
					c->_shadow->removeFromParentAndCleanup(true);
				c->stopAllActions();
				c->unscheduleAllSelectors();
				c->removeFromParentAndCleanup(true);
				if (c->getMonsterArray() && c->getMonsterArray()->count() > 0)
				{
					CCObject *pObject;
					CCARRAY_FOREACH(c->getMonsterArray(), pObject)
					{
						auto mo = (Monster *)pObject;
						CCNotificationCenter::sharedNotificationCenter()->removeAllObservers(mo);
						if (mo->_shadow)
							mo->_shadow->removeFromParentAndCleanup(true);
						mo->stopAllActions();
						mo->unscheduleAllSelectors();
						mo->removeFromParentAndCleanup(true);
					}
				}
				
				// load new char assets
				LoadLayer::perloadCharIMG(newCharName.c_str());
				// Unload old character assets if not used by the other player or AI
				auto oldCharName = c->getCharacter()->m_sString;
				if (std::find(heroVector.begin(), heroVector.end(), oldCharName) == heroVector.end())
				{
					LoadLayer::unloadCharIMG(c);
				}
			}
			auto hudLayer = gameLayer->getHudLayer();
			auto newChar = (Hero *)gameLayer->addHero(CCString::create(newCharName), c->getRole(), c->getGroup(), c->getSpawnPoint(), c->getCharNO());
			bool isPlayer = newChar->isPlayer();
			newChar->setCoin(c->getCoin());
			newChar->setCKR(c->getCKR());
			newChar->setCKR2(c->getCKR2());
			newChar->setEXP(c->getEXP());
			newChar->setLV(c->getLV());
			newChar->setKillNum(c->getKillNum());
			newChar->setWalkSpeed(224);
			newChar->setGearArray(c->getGearArray());
			newChar->changeHPbar();
			newChar->updateDataByLVOnly();
			CCObject *pObject = nullptr;
			CCARRAY_FOREACH(c->getGearArray(), pObject)
			{
				auto gear = (gearType)(((CCString *)pObject)->intValue());
				if (gear == gear00)
				{
					newChar->_isCanGear00 = true;
				}
				else if (gear == gear01)
				{
					newChar->gearCKRValue = 25;
				}
				else if (gear == gear02)
				{
					newChar->isAttackGainCKR = true;
				}
				else if (gear == gear03)
				{
					newChar->_isCanGear03 = true;
				}
				else if (gear == gear04)
				{
					if (newChar->getTempAttackValue1())
						newChar->settempAttackValue1(to_ccstring(newChar->getTempAttackValue1() + 160));
					newChar->setnAttackValue(to_ccstring(newChar->getNAttackValue() + 160));
					newChar->hasArmorBroken = true;
				}
				else if (gear == gear05)
				{
					newChar->isGearCD = true;
					newChar->_sattackcooldown1 -= 5;
					newChar->_sattackcooldown2 -= 5;
					newChar->_sattackcooldown3 -= 5;
				}
				else if (gear == gear06)
				{
					newChar->_isCanGear06 = true;
				}
				else if (gear == gear07)
				{
					newChar->gearRecoverValue = 3000;
					if (isPlayer)
					{
						hudLayer->item1Button->setCD(to_ccstring(3000));
						hudLayer->item1Button->_isColdChanged = true;
					}
				}
				else if (gear == gear08)
				{
					uint32_t tempMaxHP = newChar->getMaxHPValue();
					tempMaxHP += 6000;
					newChar->setMaxHPValue(tempMaxHP);
					newChar->hasArmor = true;
				}
			}
			newChar->_deadNum = c->_deadNum;
			newChar->_flogNum = c->_flogNum;
			newChar->isBaseDanger = c->isBaseDanger;

			if (isPlayer)
			{
				gameLayer->currentPlayer = newChar;
				// reset hud layer
				hudLayer->updateSkillButtons();
				hudLayer->resetSkillButtons();

				hudLayer->status_hpbar->setOpacity(255);
				hudLayer->status_expbar->setOpacity(255);
				hudLayer->setHPLose(newChar->getHpPercent());
			}
			else if (newChar->isCom())
			{
				newChar->doAI();
			}
			gameLayer->_CharacterArray.at(c->getCharNO() - 1) = newChar;
		}
	}

private:
	void setRandomHero(CharacterBase *c)
	{
		auto index = getIndexByHero(c);
		if (index == -1)
		{
			CCLOGERROR("Not found hero %s from hero vector", c->getCharacter()->getCString());
			return;
		}
		c->changeCharId = index;

		auto newChar = getRandomHeroExceptAll(heroVector);
		heroVector.at(index) = newChar;
	}

	inline int getIndexByHero(CharacterBase *c)
	{
		return (int)heroVector.size() < c->getCharNO() ? -1 : c->getCharNO() - 1;
	}
};
