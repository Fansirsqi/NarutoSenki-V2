#pragma once
#include "Hero.hpp"

class Mask : public Hero
{
	void perform() override
	{
		if (isCharacter("MaskRaiton"))
		{
			if (notFindFlog(0))
			{
				if (notFindHero(0))
				{
					if (notFindTower(0))
					{
						_mainTarget = nullptr;
					}
				}
			}
		}
		else
		{
			if (notFindHero(0))
			{
				if (notFindFlog(0))
				{
					if (notFindTower(0))
					{
						_mainTarget = nullptr;
					}
				}
			}
		}

		if (_mainTarget)
		{
			CCPoint moveDirection;
			CCPoint sp = getDistanceToTarget();

			if (_mainTarget->isTower())
			{
				if (abs(sp.x) > 32 || abs(sp.y) > 32)
				{
					moveDirection = ccpNormalize(sp);
					walk(moveDirection);
				}
				else
				{
					if (isFreeActionState())
					{
						changeSide(sp);
						attack(NAttack);
					}
				}
				return;
			}
			else
			{
				if (abs(sp.x) > 96 || abs(sp.y) > 16)
				{
					moveDirection = ccpNormalize(sp);
					walk(moveDirection);
					return;
				}
				else if ((abs(sp.x) > 48 || abs(sp.y) > 16) && !_isCanSkill1)
				{
					moveDirection = ccpNormalize(sp);
					walk(moveDirection);
					return;
				}
				else if (isFreeActionState())
				{
					if (_isCanSkill1 && _mainTarget->getGP() < 5000)
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
		}

		stepOn();
	}
};
