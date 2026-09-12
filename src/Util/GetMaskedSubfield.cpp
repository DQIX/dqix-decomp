#include <globaldefs.h>

// Loads a masked field through the pointer kept 8 bytes into the object.
int GetMaskedSubfield(void* obj) {
    unsigned char* field = *(unsigned char**)((char*)obj + 8);
    unsigned int value;
    if (field != NULL) {
        value = *field & 0x1f;
    } else {
        value = 0;
    }
    return value & 0xff;
}
