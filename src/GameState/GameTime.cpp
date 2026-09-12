#include "GameState.h"

// not correct, they're separate but we need the full TU before we can do this properly
extern float data_020f33b4[6];

void GameState::CalculateDeltaTime(uint64_t microseconds)
{
    if (microseconds > 50000)
        microseconds = 50000;

    numTicks_ = currentNumTicks_;
    currentNumTicks_ = 0;
    if (numTicks_ > 3)
        numTicks_ = 3;

    trueDeltaTimeMilliseconds_ = microseconds / 1000;
    effectiveDeltaTimeMilliseconds_ = trueDeltaTimeMilliseconds_ * (gameSpeed_ / 4096.0f);
    animationDeltaTime_ = 4096.0f * (effectiveDeltaTimeMilliseconds_ / 17.0f);
}

unsigned int GameState::GetEffectiveDeltaTime() const { return effectiveDeltaTimeMilliseconds_; }
unsigned int GameState::GetTrueDeltaTime() const { return trueDeltaTimeMilliseconds_; }
fix32_t GameState::GetAnimationDeltaTime() const { return animationDeltaTime_; }
unsigned int GameState::GetTickCount() const { return numTicks_; }
void GameState::SetGameSpeed(fix32_t speed) { gameSpeed_ = speed; }
fix32_t GameState::GetGameSpeed() const { return gameSpeed_; }

void GameState::AdvanceDayTimer()
{
    if (dayTimerRunning_ == 0)
        return;

    SetDayTimer(dayTimer_ + (numTicks_ * daySpeed_));
}

float GameState::GetDayTimer() const { return dayTimer_; }

void GameState::SetDayTimer(float to)
{
    if (to >= dayLength_)
    {
        int numCycles = to / dayLength_;
        to -= numCycles * dayLength_;
    }

    dayTimer_ = to;
    if (dayTimer_ >= dayLength_)
        dayTimer_ -= dayLength_;

    if (dayTimer_ >= data_020f33b4[2])
        timeOfDay_ = TimeOfDay_Evening;
    else if (dayTimer_ >= data_020f33b4[3])
        timeOfDay_ = TimeOfDay_Day;
    else if (dayTimer_ >= data_020f33b4[4])
        timeOfDay_ = TimeOfDay_Morning;
    else
        timeOfDay_ = TimeOfDay_Night;
}

void GameState::SetDayTimerRunning(CBool to) { dayTimerRunning_ = to; }
TimeOfDay GameState::GetTimeOfDay() const { return timeOfDay_; }

void GameState::SetTimeOfDay(TimeOfDay to)
{
    if ((int)to < 4)
    {
        timeOfDay_ = to;
        float thresholds[4] = {
            data_020f33b4[5],
            data_020f33b4[4],
            data_020f33b4[3],
            data_020f33b4[2]
        };
        SetDayTimer(thresholds[to]);
    }
}

bool GameState::IsMorningDayOrEvening() const { return timeOfDay_ != TimeOfDay_Night; }