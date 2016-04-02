/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "Common.h"
#include "Magic.h"
#include "Player.h"


bool LandedMagicHit( const Player::MagicAttr& magicAttr, Actor* target )
{
    int chance = 148;

    if ( (target->GetElementStrength() & magicAttr.Element) != 0 )
        chance = 0;

    if ( (target->GetElementWeakness() & magicAttr.Element) != 0 )
        chance += 40;

    chance += magicAttr.HitRate;

    if ( chance > 255 )
        chance = 255;

    chance -= target->GetMagicAbsorb();

    if ( chance < 0 )
        chance = 0;

    int r = GetNextRandom( 200 + 1 );

    return r <= chance;
}

void CalcNoneMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    result.Missed = true;
}

void CalcElemDamageMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( !target->IsAlive() )
    {
        result.Missed = true;
        return;
    }

    int damage = magicAttr.PowerStatus;

    if ( (target->GetElementStrength() & magicAttr.Element) != 0 )
    {
        damage /= 2;
    }
    else if ( (target->GetElementWeakness() & magicAttr.Element) != 0 )
    {
        damage += damage / 2;
    }

    int r = GetNextRandom( damage + 1 );

    damage += r;

    if ( LandedMagicHit( magicAttr, target ) )
        damage *= 2;

    target->AddHp( -damage );

    result.Missed = false;
    result.DealtDamage = true;
    result.Damage = damage;
}

void CalcUndeadDamageMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() && (target->GetEnemyClasses() & EnemyClass_Undead) != 0 )
    {
        CalcElemDamageMagic( magicAttr, target, result );
    }
    else
    {
        result.Missed = true;
    }
}

void CalcAddStatusMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() && LandedMagicHit( magicAttr, target ) )
    {
        target->AddStatus( magicAttr.PowerStatus );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcPowerAddStatusMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() 
        && target->GetHp() < 300 && (target->GetElementStrength() & magicAttr.Element) == 0 )
    {
        target->AddStatus( magicAttr.PowerStatus );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcXferMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() && LandedMagicHit( magicAttr, target ) )
    {
        target->SetElementStrength( 0 );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcResistMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() )
    {
        target->AddElementStrength( magicAttr.PowerStatus );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcRemoveStatusMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsStrictlyAlive() )
    {
        target->RemoveStatus( magicAttr.PowerStatus );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcSlowMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() && LandedMagicHit( magicAttr, target ) )
    {
        target->AddHitMultiplier( -1 );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcFastMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() )
    {
        target->AddHitMultiplier( 1 );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcFearMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() && LandedMagicHit( magicAttr, target ) )
    {
        target->LowerMorale( magicAttr.PowerStatus );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcCureMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( !target->IsAlive() )
    {
        result.Missed = true;
        return;
    }

    int cure = magicAttr.PowerStatus;

    int r = GetNextRandom( cure + 1 );

    cure += r;

    if ( cure > 255 )
        cure = 255;

    target->AddHp( cure );

    result.Missed = false;
    result.DealtDamage = true;
    result.Damage = -cure;
}

void CalcFogMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() )
    {
        target->AddAbsorb( magicAttr.PowerStatus );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcTmprMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() )
    {
        target->AddDamage( magicAttr.PowerStatus );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcSabrMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() )
    {
        target->AddDamage( magicAttr.PowerStatus );
        target->AddHitRate( magicAttr.HitRate );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcLockMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() && LandedMagicHit( magicAttr, target ) )
    {
        target->AddEvadeRate( -magicAttr.PowerStatus );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcInvisMagic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() )
    {
        target->AddEvadeRate( magicAttr.PowerStatus );

        result.Missed = false;
        result.DealtDamage = false;
    }
    else
        result.Missed = true;
}

void CalcCur4Magic( const Player::MagicAttr& magicAttr, Actor* target, ActionResult& result )
{
    if ( target->IsAlive() )
    {
        target->SetHp( target->GetMaxHp() );
        target->RemoveStatus( Status_AllLiving );

        result.Missed = false;
        result.DealtDamage = true;
        result.Damage = -target->GetMaxHp();
    }
    else
        result.Missed = true;
}


const CalcMagicFunc magicFuncs[] = 
{
    CalcNoneMagic,
    CalcElemDamageMagic,
    CalcUndeadDamageMagic,
    CalcAddStatusMagic,
    CalcSlowMagic,
    CalcFearMagic,
    CalcCureMagic,
    CalcCureMagic,
    CalcRemoveStatusMagic,
    CalcFogMagic,
    CalcResistMagic,
    CalcTmprMagic,
    CalcFastMagic,
    CalcSabrMagic,
    CalcLockMagic,
    CalcCur4Magic,
    CalcInvisMagic,
    CalcXferMagic,
    CalcPowerAddStatusMagic,
};


CalcMagicFunc GetMagicProc( int effect )
{
    if ( effect < 0 || effect > _countof( magicFuncs ) )
        return CalcNoneMagic;

    return magicFuncs[effect];
}
