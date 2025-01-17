#pragma once
#include "Hero.hpp"
#include "Kuchiyose/DogWall.hpp"

class Kakashi : public Hero
{
	void perform() override
	{
		_mainTarget = nullptr;
		findHeroHalf();

		if (_isCanGear06)
		{
			if ((_actionState == State::FLOAT ||
				 _actionState == State::AIRHURT ||
				 _actionState == State::HURT ||
				 _actionState == State::KNOCKDOWN) &&
				getHpPercent() < 0.5 && !_isArmored && !_isInvincible)
			{
				useGear(gear06);
			}
		}

		if (getCoinValue() >= 500 && !_isControlled && getGameLayer()->_enableGear)
		{
			if (getGearArray()->count() == 0)
				setGear(gear06);
			else if (getGearArray()->count() == 1)
				setGear(gear05);
			else if (getGearArray()->count() == 2)
				setGear(gear02);
		}

		if (checkRetri())
		{
			if (_mainTarget != nullptr)
			{
				if (stepBack2())
					return;
			}
			else
			{
				if (stepBack())
					return;
			}
		}

		if (isBaseDanger && checkBase() && !_isControlled)
		{
			bool needBack = false;
			if (isAkatsukiGroup())
			{
				if (getPositionX() < 85 * 32)
					needBack = true;
			}
			else
			{
				if (getPositionX() > 11 * 32)
					needBack = true;
			}

			if (needBack)
			{
				if (stepBack2())
					return;
			}
		}

		if (_mainTarget && _mainTarget->isNotFlog())
		{
			CCPoint moveDirection;
			CCPoint sp = getDistanceToTarget();

			if (isFreeActionState())
			{
				if (_isCanSkill3)
				{
					changeSide(sp);
					attack(SKILL3);
					return;
				}
				else if (_isCanSkill2 && _mainTarget->getGP() < 5000)
				{
					changeSide(sp);
					attack(SKILL2);
					return;
				}
				else if (enemyCombatPoint > friendCombatPoint && abs(enemyCombatPoint - friendCombatPoint) > 3000 && !_isHealling && !_isControlled)
				{
					if (abs(sp.x) < 160)
						stepBack2();
					else
						idle();
					return;
				}
				else if (abs(sp.x) < 128)
				{
					if ((abs(sp.x) > 64 || abs(sp.y) > 32))
					{
						moveDirection = ccpNormalize(sp);
						walk(moveDirection);
						return;
					}

					if (_isCanOugis2 && !_isControlled && getGameLayer()->_isOugis2Game && _mainTarget->getGP() < 5000 && !_mainTarget->_isArmored && _mainTarget->getActionState() != State::KNOCKDOWN && !_mainTarget->_isSticking)
					{
						changeSide(sp);
						attack(OUGIS2);
					}
					else if (_isCanOugis1 && !_isControlled && _mainTarget->getGP() < 5000)
					{
						changeSide(sp);
						attack(OUGIS1);
					}
					else if (_isCanSkill1 && _mainTarget->getGP() < 5000)
					{
						changeSide(sp);
						attack(SKILL1);
					}
					else
					{
						changeSide(sp);
						attack(NAttack);
					}

					return;
				}
			}
		}
		_mainTarget = nullptr;
		if (notFindFlogHalf())
			findTowerHalf();

		if (_mainTarget)
		{
			CCPoint moveDirection;
			CCPoint sp = getDistanceToTarget();

			if (abs(sp.x) > 32 || abs(sp.y) > 32)
			{
				moveDirection = ccpNormalize(sp);
				walk(moveDirection);
				return;
			}

			if (isFreeActionState())
			{
				if (_isCanSkill3 && _mainTarget->isFlog())
				{
					changeSide(sp);
					attack(SKILL3);
				}
				else if (_isCanSkill1 && _mainTarget->isFlog())
				{
					changeSide(sp);
					attack(SKILL1);
				}
				else
				{
					changeSide(sp);
					attack(NAttack);
				}
			}
			return;
		}

		if (_isHealling && getHpPercent() < 1)
		{
			if (isFreeActionState())
				idle();
		}
		else
		{
			stepOn();
		}
	}

	void changeAction() override
	{
		setSkill1Action(createAnimation(skillSPC1Array, 10.0f, false, true));
		setSkill2Action(createAnimation(skillSPC2Array, 10.0f, false, true));
		setIdleAction(createAnimation(skillSPC3Array, 5.0f, true, false));
		settempAttackValue1(to_ccstring(getSAttackValue1()));
		setsAttackValue1(getSpcAttackValue1Str());

		for (auto hero : getGameLayer()->_CharacterArray)
		{
			if (isNotSameGroupAs(hero) &&
				hero->isPlayerOrCom() &&
				hero->getActionState() != State::HURT &&
				hero->getActionState() != State::DEAD)
			{
				float distanceX = ccpSub(hero->getPosition(), getPosition()).x;
				if (distanceX < winSize.width / 2)
				{
					if (!hero->_isVisable)
					{
						if (hero->isCharacter("Konan") ||
							hero->isCharacter("Deidara"))
						{
							hero->unschedule(schedule_selector(CharacterBase::disableBuff));
						}

						hero->setOpacity(255);
						hero->setVisible(true);

						if (hero->_hpBar)
							hero->_hpBar->setVisible(true);
						if (hero->_shadow)
							hero->_shadow->setVisible(true);

						hero->_isVisable = true;
					}
				}
			}
		}

		if (isPlayer())
		{
			auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("Kakashi_skill1_1.png");
			getGameLayer()->getHudLayer()->skill1Button->setDisplayFrame(frame);
			frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("Kakashi_skill2_1.png");
			getGameLayer()->getHudLayer()->skill2Button->setDisplayFrame(frame);
		}
	}

	void resumeAction(float dt) override
	{
		setIdleAction(createAnimation(idleArray, 5.0f, true, false));
		setSkill1Action(createAnimation(skill1Array, 10.0f, false, true));
		setSkill2Action(createAnimation(skill2Array, 10.0f, false, true));

		setsAttackValue1(to_ccstring(getTempAttackValue1()));

		if (isPlayer())
		{
			auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("Kakashi_skill1.png");
			getGameLayer()->getHudLayer()->skill1Button->setDisplayFrame(frame);
			frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("Kakashi_skill2.png");
			getGameLayer()->getHudLayer()->skill2Button->setDisplayFrame(frame);
		}
		CharacterBase::resumeAction(dt);
	}

	Hero *createClone(int cloneTime) override
	{
		auto clone = create<DogWall>(CCString::create("DogWall"), CCString::create(kRoleSummon), getGroup());
		clone->setPosition(ccp(getPositionX() + (_isFlipped ? -56 : 56), getPositionY()));
		clone->setAnchorPoint(ccp(0.5f, 0.1f));
		clone->_isArmored = true;
		return clone;
	}
};
