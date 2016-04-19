/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


namespace Battle
{
    enum EncounterType;

    int FindNextActivePlayer( int prevPlayer );
    int FindPrevActivePlayer( int nextPlayer );
    void MakeDisabledPlayerActions();
    void ShuffleActors();
    EncounterType GetNextEncounterType();
    bool CanRunAway();
    void UndoAllCommands();
    void CommitCommand( int index );
    void UndoCommand( int index );
    bool TryRecoverConfuse( int actorIndex );
    void MakeConfuseAction( int actorIndex );
    void MakeEnemyAction( int actorIndex );
    int CalcNextLevelUp( int prevPlayerId );
    bool HasLost();
    bool HasWon();
    void CalcEnemyAutoHP();
    void CalcPlayerAutoHP();
    void PrepActions();
    void ResetRunningCommands();
    bool AreCommandsFinished();
    void CalcPlayerPhysDamage();
    void CalcEnemyPhysDamage();
    void CalcMagicEffect();
    void CalcItemEffect();
    bool IsMute( Party party, int index );
}
