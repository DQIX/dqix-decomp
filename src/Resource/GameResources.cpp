#include "Resource/GameResources.h"

struct Overlay17GlobalData {
    void *unknown_00;
    GameResources *gameResources;
};

extern Overlay17GlobalData *data_ov017_021d82e0;

// USA: func_ov017_0x0218b5a0
void SetGameResources(GameResources *gameResources) {
    data_ov017_021d82e0->gameResources = gameResources;
}

// USA: func_ov017_0x0218b5b0
GameResources *GetGameResources() {
    return data_ov017_021d82e0->gameResources;
}