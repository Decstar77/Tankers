#pragma once

#include "../shared/shared_game.h"

class SyncQueues {
public:
    void      Start();
    bool      CanTurn();
    void      AddTurn(i32 playerNumber, const MapTurn & turn);
    MapTurn * GetNextTurn(i32 playerNumber);
    void      FinishTurn();
    i32       GetSlidingWindowWidth() { return slidingWindowWidth; }

public:
    i32 turnRate = 24; // Per second
    f32 turnRateMS = 1 / (f32)turnRate;

private:
    i32 slidingWindowWidth = 4;
    FixedQueue<MapTurn, 10> player1Turns;
    FixedQueue<MapTurn, 10> player2Turns;
};