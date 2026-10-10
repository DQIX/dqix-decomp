#include <globaldefs.h>

void MaybeReleaseDefaultIRQSlot(void* data);

// Outer null-guarded layer over the IRQ table release routine.
void MaybeReleaseDefaultIRQSlotOuter(void* data) {
    if (data != NULL) {
        MaybeReleaseDefaultIRQSlot(data);
    }
}
