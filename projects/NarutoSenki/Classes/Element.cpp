#include "Core/Hero.hpp"
#include "Defines.h"
#include "HudLayer.h"
#include "LoadLayer.h"

HeroElement::HeroElement()
{
	rebornSprite = nullptr;
}

HeroElement::~HeroElement()
{
	CC_SAFE_RELEASE(callValue);
	CC_SAFE_RELEASE(_monsterArray);
	CC_SAFE_DELETE(skillSPC1Array);
	CC_SAFE_RELEASE(skillSPC2Array);
	CC_SAFE_RELEASE(skillSPC3Array);
	CC_SAFE_RELEASE(skillSPC4Array);
	CC_SAFE_RELEASE(skillSPC5Array);
	CC_SAFE_RELEASE(nattackArray);
	CC_SAFE_RELEASE(walkArray);
	CC_SAFE_RELEASE(knockDownArray);
	CC_SAFE_RELEASE(skill1Array);
	CC_SAFE_RELEASE(skill2Array);
	CC_SAFE_RELEASE(skill3Array);
	CC_SAFE_RELEASE(skill4Array);
	CC_SAFE_RELEASE(skill5Array);
	CC_SAFE_RELEASE(idleArray);
}

bool HeroElement::init()
{
	RETURN_FALSE_IF(!CharacterBase::init());

	setAnchorPoint(ccp(0.5, 0));
	scheduleUpdate();
	//schedule(schedule_selector(HeroElement::checkRefCount),0.5f);

	return true;
}

void HeroElement::initAction()
{
	setIdleAction(createAnimation(idleArray, 5.0f, true, false));
	setWalkAction(createAnimation(walkArray, 10.0f, true, false));
	setHurtAction(createAnimation(hurtArray, 10.0f, false, false));

	setAirHurtAction(createAnimation(airHurtArray, 10.0f, false, false));
	setKnockDownAction(createAnimation(knockDownArray, 10.0f, false, true));
	setDeadAction(createAnimation(deadArray, 10.0f, false, false));
	setFloatAction(createAnimation(floatArray, 10.0f, false, false));

	setNAttackAction(createAnimation(nattackArray, 10.0f, false, true));
	setSkill1Action(createAnimation(skill1Array, 10.0f, false, true));
	setSkill2Action(createAnimation(skill2Array, 10.0f, false, true));
	setSkill3Action(createAnimation(skill3Array, 10.0f, false, true));
	setSkill4Action(createAnimation(skill4Array, 10.0f, false, true));
	setSkill5Action(createAnimation(skill5Array, 10.0f, false, true));
}

void HeroElement::setShadows()
{
	if (!_shadow)
	{
		_shadow = CCSprite::createWithSpriteFrameName("shadows");
		_shadow->setAnchorPoint(ccp(0.5, 0.5));
		_shadow->setPosition(getPosition());
		getGameLayer()->shadowBatch->addChild(_shadow);
	}
}

void HeroElement::setHPbar()
{
	if (getGameLayer()->playerGroup != getGroup()->getCString())
	{
		_hpBar = HPBar::create("hp_bar_r.png");
	}
	else if (isCom() || isClone() || isKugutsu() || isSummon())
	{
		_hpBar = HPBar::create("hp_bar_b.png");
	}
	else if (isPlayer())
	{
		_hpBar = HPBar::create("hp_bar.png");
	}
	_hpBar->setPositionY(getHeight());
	_hpBar->setDelegate(this);
	addChild(_hpBar);
	changeHPbar();
}

void HeroElement::changeHPbar()
{
	if (_exp >= 500 && _level == 1)
	{
		_level = 2;
		uint32_t newValue = getCkrValue() + 15000;
		setCkrValue(newValue);
		_isCanOugis1 = true;
		if (isPlayer())
		{
			getGameLayer()->setCKRLose(false);
			getGameLayer()->removeOugisMark(1);
		}
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 500;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 9));
		_rebornTime += 1;
	}
	else if (_exp >= 1000 && _level == 2)
	{
		_level = 3;
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 1000;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 18));
		_rebornTime += 2;
	}
	else if (_exp >= 1500 && _level == 3)
	{
		_level = 4;
		uint32_t newValue = getCkr2Value() + 25000;
		setCkr2Value(newValue);
		_isCanOugis2 = true;
		if (isPlayer())
		{
			getGameLayer()->setCKRLose(true);
			getGameLayer()->removeOugisMark(2);
		}
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 2000;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 27));
		_rebornTime += 3;
	}
	else if (_exp >= 2000 && _level == 4)
	{
		_level = 5;
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 2500;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 36));
		_rebornTime += 4;
	}
	else if (_exp >= 2500 && _level == 5)
	{
		_level = 6;
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 3000;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 45));
		_rebornTime += 5;
	}

	if (_hpBar)
	{
		auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(CCString::createWithFormat("hp_bottom%d.png", int(_level))->getCString());
		_hpBar->getHPBottom()->setDisplayFrame(frame);
	}
}

void HeroElement::checkRefCount(float dt)
{
	CCLOG("[Ref Check] %s has %d references", getCharacter()->getCString(), callValue->retainCount());
}

void HeroElement::dealloc()
{
	stopAllActions();
	_actionState = State::DEAD;

	if (isNotCharacter("Minato"))
	{
		if (hasMonsterArrayAny())
		{
			CCObject *pObject;
			CCARRAY_FOREACH(getMonsterArray(), pObject)
			{
				auto mo = (CharacterBase *)pObject;

				std::erase(getGameLayer()->_CharacterArray, mo);

				CCNotificationCenter::sharedNotificationCenter()->removeObserver(mo, "acceptAttack");
				mo->stopAllActions();
				mo->unscheduleAllSelectors();
				mo->setActionState(State::DEAD);
				mo->removeFromParent();
				mo = nullptr;
			}
			getMonsterArray()->removeAllObjects();
			_monsterArray = nullptr;
		}
	}

	if (isClone() || isKugutsu() || isSummon())
	{
		unschedule(schedule_selector(CharacterBase::setAI));
		CCNotificationCenter::sharedNotificationCenter()->removeObserver(this, "acceptAttack");

		std::erase(getGameLayer()->_CharacterArray, this);

		if (_master && _master->getMonsterArray())
		{
			int index = _master->getMonsterArray()->indexOfObject(this);
			if (index >= 0)
			{
				_master->getMonsterArray()->removeObjectAtIndex(index);
			}
		}

		if (isCharacter("NarakaPath"))
		{
			_master->_skillChangeBuffValue = 0;

			if (_master->isPlayer())
			{
				getGameLayer()->getHudLayer()->skill5Button->unLock();
			}
		}
		else if (isCharacter("Akamaru",
							 "Karasu",
							 "Parents"))
		{
			_master->setActionResume();
		}
		else if (isCharacter("Sanshouuo"))
		{
			if (_master->isPlayer())
			{
				getGameLayer()->getHudLayer()->skill4Button->unLock();
			}
		}
		else if (isCharacter("MaskFuton",
							 "MaskRaiton",
							 "MaskKaton"))
		{
			if (_master->hearts > 0)
			{
				if (_master->isPlayer())
				{
					getGameLayer()->getHudLayer()->skill4Button->unLock();
				}
			}
		}
		else if (isCharacter("Saso"))
		{
			if (_master->isPlayer())
			{
				getGameLayer()->getHudLayer()->skill5Button->unLock();
			}
		}

		removeFromParentAndCleanup(true);
	}
	else
	{
		if (isCharacter("Kankuro"))
		{
			if (isPlayer())
			{
				getGameLayer()->getHudLayer()->skill4Button->unLock();
				getGameLayer()->getHudLayer()->skill5Button->unLock();
			}
		}
		else if (isCharacter("Shikamaru", "Choji"))
		{
			for (auto hero : getGameLayer()->_CharacterArray)
			{
				if (hero->_isSticking)
				{
					if (hero->getActionState() != State::DEAD)
					{
						hero->removeLostBlood(0.1f);
						hero->idle();
					}
				}
			}
		}
		else if (isCharacter("Hidan"))
		{
			if (isPlayer())
			{
				getGameLayer()->getHudLayer()->skill1Button->unLock();
			}
		}

		if (isNotGuardian())
		{
			if (rebornLabelTime == 3)
			{
				scheduleOnce(schedule_selector(HeroElement::reborn), 3.0f);
			}
			else
			{
				rebornLabelTime = getRebornTime();
				scheduleOnce(schedule_selector(HeroElement::reborn), getRebornTime());
			}
			if (!rebornSprite)
			{
				rebornSprite = CCSprite::create();
				CCSprite *skullSpirte = CCSprite::createWithSpriteFrameName("skull.png");
				skullSpirte->setPosition(ccp(0, 0));
				rebornSprite->addChild(skullSpirte);

				rebornLabel = CCLabelBMFont::create(to_cstr(rebornLabelTime), "Fonts/1.fnt");
				rebornLabel->setScale(0.3f);
				rebornLabel->setPosition(ccp(skullSpirte->getContentSize().width, 0));
				rebornSprite->addChild(rebornLabel);

				rebornSprite->setPosition(ccp(getContentSize().width / 2, getContentSize().height / 2));
				addChild(rebornSprite);
			}
			schedule(schedule_selector(HeroElement::countDown), 1);
		}
	}
}

void HeroElement::countDown(float dt)
{
	rebornLabelTime -= 1;
	rebornLabel->setString(to_cstr(rebornLabelTime));
}

void HeroElement::reborn(float dt)
{
	if (!enableReborn)
	{
		// TODO: Spectating Mode
		// if (isPlayer())
		// getGameLayer()->getHudLayer()->enableSpectatingMode();
		return;
	}

	CharacterBase::reborn(dt);
	// If the character has changed, then cleanup and return
	if (changeCharId > -1)
	{
		if (rebornSprite)
		{
			rebornSprite->removeFromParent();
			rebornSprite = nullptr;
		}
		checkRefCount(0);
		return;
	}

	setPosition(getSpawnPoint());

	if (getPosition().equals(getSpawnPoint()))
	{
		CCNotificationCenter::sharedNotificationCenter()->addObserver(this, callfuncO_selector(CharacterBase::acceptAttack), "acceptAttack", nullptr);
		setOpacity(255);
		setHPValue(getMaxHPValue(), false);
		setHPbar();
		_actionState = State::HURT;

		if (getLV() < 4)
		{
			if (isKonohaGroup())
				setEXP(getEXP() + getGameLayer()->kEXPBound);
			else
				setEXP(getEXP() + getGameLayer()->aEXPBound);

			changeHPbar();

			if (isPlayer())
			{
				getGameLayer()->getHudLayer()->setEXPLose();
			}
		}

		if (isKonohaGroup())
		{
			if (_isFlipped)
			{
				setFlipX(false);
				_isFlipped = false;
			}
		}
		else
		{
			if (!_isFlipped)
			{
				setFlipX(true);
				_isFlipped = true;
			}
		}
		idle();
		if (rebornSprite)
		{
			unschedule(schedule_selector(HeroElement::countDown));
			rebornSprite->removeFromParent();
			rebornSprite = nullptr;
		}
		if (isNotPlayer())
		{
			doAI();
		}
		else
		{
			if (_isAI)
			{
				doAI();
			}
			getGameLayer()->getHudLayer()->status_hpbar->setOpacity(255);
			getGameLayer()->setHPLose(getHpPercent());
		}
		scheduleUpdate();
	}
	getGameLayer()->reorderChild(this, -getPositionY());
}

/*---------------
init Monster
---------------*/

Monster::Monster()
{
}

Monster::~Monster()
{
	CC_SAFE_RELEASE(callValue);
}

bool Monster::init()
{
	RETURN_FALSE_IF(!CharacterBase::init());

	setAnchorPoint(ccp(0.5, 0));
	scheduleUpdate();

	return true;
}

void Monster::setID(CCString *character, CCString *role, CCString *group)
{
	setCharacter(character);
	setRole(role);
	setGroup(group);

	CCArray *animationArray = CCArray::create();
	const char *filePath;

	filePath = CCString::createWithFormat("Element/Monster/%s.xml", getCharacter()->getCString())->getCString();

	// string key =KTools::getKeycode(filePath);

	KTools::readXMLToArray(filePath, animationArray);

	//init Attribute; & indleFrame

	CCArray *tmpAction = (CCArray *)(animationArray->objectAtIndex(0));
	CCArray *tmpData = (CCArray *)(tmpAction->objectAtIndex(0));
	idleArray = (CCArray *)(tmpAction->objectAtIndex(1));

	CCString *tmpName;
	CCString *tmpHpMax;
	int tmpWidth;
	int tmpHeight;
	uint32_t tmpSpeed;
	int tmpCombatPoint;

	readData(tmpData, tmpName, tmpHpMax, tmpWidth, tmpHeight, tmpSpeed, tmpCombatPoint);
	setMaxHPValue(tmpHpMax->uintValue(), false);
	setHPValue(getMaxHPValue(), false);

	setHeight(tmpHeight);
	setWalkSpeed(tmpSpeed);

	if (!getCKR() && !getCKR2())
	{
		setCkrValue(0);
		setCkr2Value(0);
	}

	//init WalkFrame
	tmpAction = (CCArray *)(animationArray->objectAtIndex(1));
	walkArray = (CCArray *)(tmpAction->objectAtIndex(1));

	//init DeadFrame
	tmpAction = (CCArray *)(animationArray->objectAtIndex(6));
	deadArray = (CCArray *)(tmpAction->objectAtIndex(1));

	//init nAttack data & Frame Array
	tmpAction = (CCArray *)(animationArray->objectAtIndex(7));
	tmpData = (CCArray *)(tmpAction->objectAtIndex(0));
	uint32_t tmpCD;
	CCString *tmpValue;
	readData(tmpData, _nattackType, tmpValue, _nattackRangeX, _nattackRangeY, tmpCD, tmpCombatPoint);
	setnAttackValue(tmpValue);
	nattackArray = (CCArray *)(tmpAction->objectAtIndex(1));

	setCoinValue(50);

	initAction();
}

void Monster::initAction()
{
	setIdleAction(createAnimation(idleArray, 5.0, true, false));
	setWalkAction(createAnimation(walkArray, 10.0, true, false));
	setDeadAction(createAnimation(deadArray, 10.0f, false, false));
	if (isCharacter("Kage",
					"KageHand",
					"FutonSRK",
					"FutonSRK2",
					"Kubi"))
	{
		setNAttackAction(createAnimation(nattackArray, 10.0, false, false));
	}
	else
	{
		setNAttackAction(createAnimation(nattackArray, 10.0, false, true));
	}
}

void Monster::setHPbar()
{
	if (getGameLayer()->playerGroup != getGroup()->getCString())
	{
		_hpBar = HPBar::create("hp_bar_r.png");
	}
	else
	{
		_hpBar = HPBar::create("hp_bar_b.png");
	}

	_hpBar->setPositionY(getHeight());
	_hpBar->setDelegate(this);
	addChild(_hpBar);
	changeHPbar();
}

void Monster::changeHPbar()
{
	if (_exp >= 500 && _level == 1)
	{
		_level = 2;
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 1000;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 9));
	}
	else if (_exp >= 1000 && _level == 2)
	{
		_level = 3;
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 1500;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 18));
	}
	else if (_exp >= 1500 && _level == 3)
	{
		_level = 4;
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 2000;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 27));
	}
	else if (_exp >= 2000 && _level == 4)
	{
		_level = 5;
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 2500;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 36));
	}
	else if (_exp >= 2500 && _level == 5)
	{
		_level = 6;
		uint32_t tempMaxHP = getMaxHPValue();
		tempMaxHP += 3000;
		setMaxHPValue(tempMaxHP);
		setnAttackValue(to_ccstring(getNAttackValue() + 48));
	}

	if (_hpBar)
	{
		auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(CCString::createWithFormat("hp_bottom%d.png", int(_level))->getCString());
		_hpBar->getHPBottom()->setDisplayFrame(frame);
	}
}

void Monster::setAI(float dt)
{
	auto charName = getCharacter()->getCString();
	if (is_same(charName, "Kage"))
	{
		for (auto hero : getGameLayer()->_CharacterArray)
		{
			if (strcmp(getGroup()->getCString(), hero->getGroup()->getCString()) != 0 &&
				hero->getActionState() != State::DEAD &&
				hero->getActionState() != State::O2ATTACK &&
				!hero->_isInvincible &&
				!hero->_isArmored &&
				hero->_isVisable)
			{
				CCPoint sp = ccpSub(hero->getPosition(), getPosition());
				float distanceY = hero->_originY ? abs(getPositionY() - hero->_originY) : abs(sp.y);
				float distanceX = _isFlipped ? hero->getPositionX() - getPositionX() + getContentSize().width : hero->getPositionX() - getPositionX() - getContentSize().width;
				if (abs(distanceX) < 32 && distanceY < 48)
				{
					if (!_monsterArray)
					{
						auto dic = CCDictionary::create();
						CCString *monterName = CCString::create("KageHand");
						dic->setObject(monterName, 1);
						setMon(this, (void *)dic);
						unschedule(schedule_selector(CharacterBase::setAI));
					}
				}
			}
		}
		return;
	}
	else if (is_same(charName, "Mouse"))
	{
		if (notFindHero(0))
			_mainTarget = nullptr;
	}
	else if (is_same(charName, "Spider") ||
			 is_same(charName, "ClayBird"))
	{
		if (notFindHero(0))
		{
			if (notFindTower(0))
				_mainTarget = nullptr;
		}
	}
	else if (is_same(charName, "FutonSRK2") ||
			 is_same(charName, "FutonSRK"))
	{
	}
	else
	{
		if (notFindHero(0))
		{
			if (notFindFlog(0))
				_mainTarget = nullptr;
		}
	}

	CCPoint moveDirection;

	if (_mainTarget)
	{
		CCPoint sp;

		sp = ccpSub(ccp(_mainTarget->getPositionX(), _mainTarget->_originY ? _mainTarget->_originY : _mainTarget->getPositionY()),
					ccp(getPositionX(), _originY ? _originY : getPositionY()));

		if (is_same(charName, "FutonSRK2") ||
			is_same(charName, "FutonSRK"))
		{
			if (abs(sp.x) > 48 || abs(sp.y) > 32)
			{
				setActionState(State::WALK);
				moveDirection = ccpNormalize(sp);
				walk(moveDirection);
				return;
			}

			if (_mainTarget->isPlayerOrCom())
			{
				stopAllActions();
				dealloc();
			}
		}
		else if (is_same(charName, "Traps"))
		{
			if (abs(sp.x) > 32 || abs(sp.y) > 32)
			{
			}
			else
			{
				attack(NAttack);
			}
		}
		else if (abs(sp.x) > 32 || abs(sp.y) > 16)
		{
			if (!is_same(charName, "Dogs") &&
				!is_same(charName, "Mine") &&
				!is_same(charName, "Traps") &&
				!is_same(charName, "Yominuma") &&
				!is_same(charName, "Tsukuyomi"))
			{
				moveDirection = ccpNormalize(sp);

				walk(moveDirection);
			}
		}
		else
		{
			if (is_same(charName, "Mine"))
			{
				if (_mainTarget->isPlayerOrCom())
				{
					attack(NAttack);
					unschedule(schedule_selector(CharacterBase::setAI));
				}
			}
			else
			{
				changeSide(sp);
				attack(NAttack);

				if (!is_same(charName, "SmallSlug"))
				{
					unschedule(schedule_selector(CharacterBase::setAI));
				}
			}
		}
	}
	else
	{
		if (is_same(charName, "Rocket") ||
			is_same(charName, "Spider") ||
			is_same(charName, "ClayBird"))
		{
			stepOn();
		}
		else
		{
			idle();
		}
	}
}

void Monster::dealloc()
{
	unschedule(schedule_selector(CharacterBase::setAI));
	setActionState(State::DEAD);
	stopAllActions();

	if (isCharacter("FutonSRK", "FutonSRK2"))
	{
		auto call = CCCallFunc::create(this, callfunc_selector(Monster::dealloc2));
		auto seqArray = CCArray::create();
		seqArray->addObject(getDeadAction());
		seqArray->addObject(call);
		auto seq = CCSequence::create(seqArray);
		runAction(seq);
		return;
	}
	if (isCharacter("HiraishinMark"))
	{
		_master->_isCanSkill1 = false;
		_master->setActionResume();
		_master->scheduleOnce(schedule_selector(CharacterBase::enableSkill1), _sattackcooldown1);

		if (_master->isPlayer())
		{
			auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("Minato_skill1.png");
			getGameLayer()->getHudLayer()->skill1Button->setDisplayFrame(frame);
			getGameLayer()->getHudLayer()->skill1Button->_clickNum++;
			getGameLayer()->clearDoubleClick();
		}

		CCObject *pObject;
		CCARRAY_FOREACH(_master->getMonsterArray(), pObject)
		{
			auto mo = (Monster *)pObject;

			if (mo->isCharacter("HiraishinMark"))
			{
				int index = _master->getMonsterArray()->indexOfObject(mo);
				_master->getMonsterArray()->removeObjectAtIndex(index);
				mo->removeFromParentAndCleanup(true);
			}
		}

		return;
	}

	if (isCharacter("SmallSlug"))
	{
		if (_secmaster && _secmaster->getMonsterArray())
		{
			int index = _secmaster->getMonsterArray()->indexOfObject(this);
			if (index >= 0)
			{
				_secmaster->getMonsterArray()->removeObjectAtIndex(index);
			}
		}
	}
	else
	{
		if (_master && _master->getMonsterArray())
		{
			int index = _master->getMonsterArray()->indexOfObject(this);
			if (index >= 0)
			{
				_master->getMonsterArray()->removeObjectAtIndex(index);
			}
		}
	}

	if (isCharacter("KageHand", "Kage"))
	{
		auto call = CCCallFunc::create(this, callfunc_selector(Monster::dealloc2));

		auto seqArray = CCArray::create();
		seqArray->addObject(getDeadAction());
		seqArray->addObject(call);

		auto call2 = CCCallFunc::create(this, callfunc_selector(Monster::setResume));
		seqArray->addObject(call2);

		auto seq = CCSequence::create(seqArray);
		runAction(seq);
	}
	else
	{
		removeFromParentAndCleanup(true);
	}
}

void Monster::setDirectMove(int length, float delay, bool isReverse)
{
	CCPoint direction = ccp(_isFlipped ? getPosition().x - length : getPosition().x + length, getPositionY());
	CCPoint direction2 = getPosition();
	auto mv = CCMoveTo::create(delay, direction);
	auto call = CCCallFunc::create(this, callfunc_selector(Monster::dealloc));
	CCAction *seq;
	if (!isReverse)
	{
		seq = CCSequence::create(mv, call, nullptr);
	}
	else
	{
		auto mv2 = CCMoveTo::create(delay, direction2);
		seq = CCSequence::create(mv, mv2, call, nullptr);
	}

	runAction(seq);
}

void Monster::setEaseIn(int length, float delay)
{
	CCPoint direction = ccp(_isFlipped ? getPosition().x - length : getPosition().x + length,
							getPositionY());
	auto mv = CCMoveTo::create(1.0f, direction);
	auto eo = CCEaseIn::create(mv, delay);

	auto call = CCCallFunc::create(this, callfunc_selector(Monster::dealloc));
	auto seq = CCSequence::create(eo, call, nullptr);
	runAction(seq);
}

void Monster::setDirectMoveBy(int length, float delay)
{
	CCPoint direction = ccp(_isFlipped ? getPosition().x - length : getPosition().x + length,
							getPositionY());

	if (_mainTarget)
	{
		auto mv = CCMoveBy::create(0.1f, ccp(_mainTarget->getPositionX() > getPositionX() ? 16 : -16, _mainTarget->getPositionY() > getPositionY() ? 16 : -16));

		runAction(CCRepeatForever::create(mv));
		_mainTarget = nullptr;
		_master->_mainTarget = nullptr;
	}
	else
	{
		auto mv = CCMoveBy::create(0.1f, ccp(_isFlipped ? -16 : 16, 0));

		runAction(CCRepeatForever::create(mv));
	}

	auto delayTime = CCDelayTime::create(delay);
	auto call = CCCallFunc::create(this, callfunc_selector(Monster::dealloc));
	auto seq = CCSequence::create(delayTime, call, nullptr);
	runAction(seq);
}

void Monster::setResume()
{
	if (getSecMaster())
	{
		getSecMaster()->dealloc();
	}
	else
	{
		if (_master && _master->isCharacter("Shikamaru"))
		{
			_master->idle();
		}
	}
}

void Monster::dealloc2()
{
	removeFromParentAndCleanup(true);
}
