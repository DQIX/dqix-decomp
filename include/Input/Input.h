#pragma once

#define KEY_A 0x1
#define KEY_B 0x2
#define KEY_SELECT 0x4
#define KEY_START 0x8
#define KEY_RIGHT 0x10
#define KEY_LEFT 0x20
#define KEY_UP 0x40
#define KEY_DOWN 0x80
#define KEY_R 0x100
#define KEY_L 0x200
#define KEY_X 0x400
#define KEY_Y 0x800

class KeyManager
{
public:
    unsigned short currentKeys_;
    unsigned short previousKeys_;

    // these hold values 1,2,3,4,5,6,7,0,1000. Not sure if they're
    // ever used
    unsigned short unknown_4_;
    unsigned short unknown_6_;
    unsigned short unknown_8_;
    unsigned short unknown_a_;
    unsigned short unknown_c_;
    unsigned short unknown_e_;
    unsigned short unknown_10_;
    unsigned short unknown_12_;
    unsigned short unknown_14_;

    char pad_16[2];
    unsigned int keyRepeatCountdown_;
    unsigned char receivingInput_;
    char pad_1d[3];

    void Initialize();

    // (blank) usa: func_02012298
    KeyManager();

    void RefreshKeys();

    // (blank) usa: func_0201238c
    ~KeyManager();

    bool IsLeftHeld() const;
    bool IsRightHeld() const;
    bool IsUpHeld() const;
    bool IsDownHeld() const;

    bool IsAHeld() const;
    bool IsBHeld() const;
    bool IsXHeld() const;
    bool IsYHeld() const;

    // Returns true if any of the masked keys are currently held
    bool TestKeysHeld(unsigned short mask) const;
    // For a single key, returns true if it was just pressed this frame.
    // For multiple, returns true if none were down last frame and now at least one is
    bool TestKeysPressed(unsigned short mask) const;
    // For a single key, returns true if it was just released this frame.
    // For multiple, returns true if at least one was down last frame and now none are
    bool TestKeysReleased(unsigned short mask) const;
    // Mimics key repeat behavior on keyboards (holding a button down activates it once,
    // then repeatedly activates it again after a delay). This continues while any
    // key in the mask is held
    bool TestKeyRepeat(unsigned short mask);

    void CancelCurrentKeyRepeat();
    // Clears the current and previous mask, so that you have to release and press again
    // for anything to register
    void Clear();

    void SetInputEnabled(bool enabled);
};