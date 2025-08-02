#ifndef M_CHEAT_H
#define M_CHEAT_H

#include "d_event.h"

boolean C_Responder (event_t *ev);

typedef enum {
    CHEAT_IDDQD_GOD,
    CHEAT_IDKFA_GIVE_ALL,
    CHEAT_CHOPPERS_CHAINSAW,
    CHEAT_IDRATE_FPS,
    CHEAT_ROCKETS_ENABLE
} FAST_CHEAT_T;

void Apply_Cheat(FAST_CHEAT_T cheat);

#endif // M_CHEAT_H
