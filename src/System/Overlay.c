#include <globaldefs.h>

/* TODO: better */
#define FALSE 0
#define TRUE 1
#define BOOL int

struct Unk_data_01ffd34c {
    unsigned int unk_00[6];
};

struct Unk_data_01ffd394 {
    unsigned int unk_00[8];
    unsigned int unk_20;
    unsigned int unk_24;
    unsigned int unk_28;
};

struct Unk_data_020e8f20 {
    unsigned int slot;
    void* unk_04;
};

extern struct Unk_data_01ffd34c data_01ffd34c;
extern unsigned char data_01ffd344;
extern unsigned char data_01ffd340;
extern signed char data_01ffd364[8][6];
extern struct Unk_data_01ffd394 data_01ffd394[0x23];
extern int data_01ffd348;

extern struct Unk_data_020e8f20 const data_020e8f20[0x23];

extern void func_020c805c(struct Unk_data_01ffd34c*);
extern void func_020a18d4(signed char*);
extern void func_020cd0f4(struct Unk_data_01ffd394*, int, int);

extern void func_020a1bb4(unsigned int overlayID);
extern void func_020d9834(int arg_0);
extern void func_020d970c(void);
extern void func_020cd27c(struct Unk_data_01ffd394*);
extern void func_020cd3a0(struct Unk_data_01ffd394*);
extern void func_020d974c(void);
extern void func_020c8074(struct Unk_data_01ffd34c*);
extern void func_020c80f8(struct Unk_data_01ffd34c*);

void func_020a1838(void) {
    func_020c805c(&data_01ffd34c);

    data_01ffd344 = 1;
    data_01ffd340 = 0;

    {
        unsigned int i;
        for (i = 0; i < 8; i++) {
            func_020a18d4(data_01ffd364[i]);
        }
    }

    {
        unsigned int i;
        for (i = 0; i < 0x23; i++) {
            func_020cd0f4(&data_01ffd394[i], 0, i);
        }
    }

    data_01ffd348 = 0;
}

void func_020a18d4(signed char* slots) {
    unsigned int i = 0;

    for (i = 0; i < 6; i++)
        slots[i] = -1;
}

BOOL func_020a18f4(unsigned int overlayID) {
    int result = FALSE;

    if (overlayID < 0x23) {
        result = data_01ffd364[data_01ffd348][data_020e8f20[overlayID].slot] == overlayID;
    }

    return result;
}

/*
 * func_020a1940: <https://decomp.me/scratch/50gQu> (LoadOverlay)
 * func_020a1a40: <https://decomp.me/scratch/9QbXe> (LoadOverlay2)
 */
