/*
   Copyright 2012 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#pragma once


namespace Battle
{
    enum EncounterType;
    struct Command;

    int FindNextActivePlayer( int prevPlayer );
    int FindPrevActivePlayer( int nextPlayer );
    void MakeDisabledPlayerActions( Command* commands );
    EncounterType GetNextEncounterType();
    bool CanRunAway( const Command& curCmd );
    void UndoAllCommands( Command* commands );
    void CommitCommand( Command& cmd );
    void UndoCommand( Command& cmd );
    bool TryRecoverConfuse( int enemyId );
    void MakeConfuseAction( int enemyId, Command& curCmd );
    void MakeEnemyAction( int enemyId, Command& curCmd );
    int CalcNextLevelUp( int prevPlayerId );
    bool HasLost();
    bool HasWon();
    void CalcEnemyAutoHP( ActionResult* actionResults, int& resultCount );
    void CalcPlayerAutoHP( ActionResult* actionResults, int& resultCount );
    void CalcPlayerPhysDamage( const Command& curCmd, ActionResult& strikeResult, int& resultCount );
    void CalcEnemyPhysDamage( const Command& curCmd, ActionResult& strikeResult, int& resultCount );
    void CalcMagicEffect( const Command& curCmd, ActionResult* actionResults, int& resultCount );
    void CalcItemEffect( const Command& curCmd, ActionResult* actionsResults, int& resultCount );
    bool IsMute( Party party, int index );
}
