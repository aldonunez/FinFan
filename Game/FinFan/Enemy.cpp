/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Battle.h"


namespace Battle
{
    int Enemy::GetHp()
    {
        return Hp;
    }

    void Enemy::SetHp( int value )
    {
        Hp = value;
    }

    int Enemy::GetStatus()
    {
        return Status;
    }

    void Enemy::SetStatus( int value )
    {
        Status = value;
    }

    int Enemy::GetHitMultiplier()
    {
        return HitMultiplier;
    }

    void Enemy::SetHitMultiplier( int value )
    {
        HitMultiplier = value;
    }

    int Enemy::GetMorale()
    {
        return Morale;
    }

    void Enemy::SetMorale( int value )
    {
        Morale = value;
    }

    int Enemy::GetAbsorb()
    {
        return Absorb;
    }

    void Enemy::SetAbsorb( int value )
    {
        Absorb = value;
    }

    int Enemy::GetEvadeRate()
    {
        return EvadeRate;
    }

    void Enemy::SetEvadeRate( int value )
    {
        EvadeRate = value;
    }

    int Enemy::GetElementStrength()
    {
        return ElemStrength;
    }

    void Enemy::SetElementStrength( int value )
    {
        ElemStrength = value;
    }

    int Enemy::GetDamage()
    {
        return Damage;
    }

    void Enemy::SetDamage( int value )
    {
        Damage = value;
    }

    int Enemy::GetHitRate()
    {
        return HitRate;
    }

    void Enemy::SetHitRate( int value )
    {
        HitRate = value;
    }


    int Enemy::GetElementWeakness()
    {
        return enemyAttrs[Type].ElementWeakness;
    }

    int Enemy::GetMagicAbsorb()
    {
        return enemyAttrs[Type].MagicDefense;
    }

    int Enemy::GetEnemyClasses()
    {
        return enemyAttrs[Type].Classes;
    }

    int Enemy::GetMaxHp()
    {
        return enemyAttrs[Type].Hp;
    }

    int Enemy::GetCritHitRate()
    {
        return enemyAttrs[Type].CritHitRate;
    }

    int Enemy::GetMaxHits()
    {
        return enemyAttrs[Type].Hits;
    }

    int Enemy::GetTargetClasses()
    {
        return 0;
    }

    int Enemy::GetTargetElement()
    {
        return enemyAttrs[Type].AttackElement;
    }

    int Enemy::GetTargetStatus()
    {
        return enemyAttrs[Type].Status;
    }
}
