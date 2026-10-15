#include "Input/Input.h"
#include "System/Input.h"
#include "Resource/Brightness.h"
#include <std_library_functions.h>

unsigned short ComputeCurrentKeys();

extern "C"
{
    // get signed brightness
    int func_020c39c8(volatile unsigned short*);

    void* func_0205ec34();
    // set flag in some big flag mask
    void func_0206df6c(void*, void*, int, bool);
}

void KeyManager::Initialize()
{
    unknown_4_ = 1;
    unknown_6_ = 2;
    unknown_8_ = 3;
    unknown_a_ = 4;
    unknown_c_ = 5;
    unknown_e_ = 6;
    unknown_10_ = 7;
    unknown_12_ = 0;
    unknown_14_ = 1000;
    receivingInput_ = true;
    keyRepeatCountdown_ = 0;
}

KeyManager::KeyManager()
{
}

void KeyManager::RefreshKeys()
{
    bool ignoreInput = false;
    int subBrightness, mainBrightness;

    if (!receivingInput_ || Arm7IsLidDown())
        goto do_ignore;

    subBrightness = func_020c39c8(&REG_MASTER_BRIGHT_SUB);
    mainBrightness = func_020c39c8(&REG_MASTER_BRIGHT);
    if (abs(mainBrightness) + abs(subBrightness) < 0x20)
        goto dont_ignore;

do_ignore:
    ignoreInput = true;
dont_ignore:
    if (ignoreInput)
    {
        previousKeys_ = 0;
        currentKeys_ = 0;
    }
    else
    {
        previousKeys_ = currentKeys_;
        currentKeys_ = ComputeCurrentKeys();
        if (currentKeys_ != 0)
        {
            void* flagManager = func_0205ec34();
            func_0206df6c(flagManager, (unsigned char*)flagManager + 0x8c, 0x113e, true);
        }
    }
}

unsigned short ComputeCurrentKeys()
{
    return ((REG_KEYINPUT | ARM7_KEYINDATA) ^ 0x2fff) & 0x2fff;
}

KeyManager::~KeyManager()
{
}

bool KeyManager::IsLeftHeld() const 
{
    return currentKeys_ & KEY_LEFT;
}

bool KeyManager::IsRightHeld() const
{
    return currentKeys_ & KEY_RIGHT;
}

bool KeyManager::IsUpHeld() const   
{
    return currentKeys_ & KEY_UP;
}

bool KeyManager::IsDownHeld() const 
{
    return currentKeys_ & KEY_DOWN;
}

bool KeyManager::IsAHeld() const
{
    return currentKeys_ & KEY_A;
}

bool KeyManager::IsBHeld() const
{
    return currentKeys_ & KEY_B;
}

bool KeyManager::IsXHeld() const
{
    return currentKeys_ & KEY_X;
}

bool KeyManager::IsYHeld() const
{
    return currentKeys_ & KEY_Y;
}

bool KeyManager::TestKeysHeld(unsigned short mask) const
{
    if (currentKeys_ & mask)
        return true;
    return false;
}

bool KeyManager::TestKeysPressed(unsigned short mask) const
{
    if ((currentKeys_ & mask) && !(previousKeys_ & mask))
        return true;
    return false;
}

bool KeyManager::TestKeysReleased(unsigned short mask) const
{
    if (!(currentKeys_ & mask) && (previousKeys_ & mask))
        return true;
    return false;
}

bool KeyManager::TestKeyRepeat(unsigned short mask)
{
    if (TestKeysHeld(mask))
    {
        unsigned int countdown = keyRepeatCountdown_;
        if (countdown == 0)
        {
            if (TestKeysPressed(mask))
            {
                keyRepeatCountdown_ = 8;
                return true;
            }
            else
            {
                keyRepeatCountdown_ = 0;
                return true;
            }
        }
        keyRepeatCountdown_ = countdown - 1;
    }
    else if (!TestKeysHeld(~KEY_START))
    {
        keyRepeatCountdown_ = 0;
    }
    return false;
}

void KeyManager::CancelCurrentKeyRepeat()
{
    keyRepeatCountdown_ = -1;
}

void KeyManager::Clear()
{
    currentKeys_ = previousKeys_ = 0;
}

void KeyManager::SetInputEnabled(bool enabled)
{
    receivingInput_ = enabled;
}