/* ============================================================
   stack.c - array-backed LIFO for mission history / undo
   ------------------------------------------------------------
   Every dispatched mission is pushed here. "Undo Last Mission"
   is a pop of this stack - undo is inherently last-action-first,
   so LIFO is the correct structure, not a coincidental choice.
   ============================================================ */

#include <stdio.h>
#include "logistics_internal.h"

int stack_is_empty(LogisticsManager *lm) {
    return lm->historyTop == -1;
}

int stack_is_full(LogisticsManager *lm) {
    return lm->historyTop == LOGISTICS_MAX_HISTORY - 1;
}

int stack_push(LogisticsManager *lm, MissionRecord m) {
    if (stack_is_full(lm)) return 0;

    lm->historyTop++;
    lm->historyArr[lm->historyTop] = m;
    return 1;
}

int stack_pop(LogisticsManager *lm, MissionRecord *out) {
    if (stack_is_empty(lm)) return 0;

    *out = lm->historyArr[lm->historyTop];
    lm->historyTop--;
    return 1;
}

void stack_display(LogisticsManager *lm) {
    if (stack_is_empty(lm)) {
        printf("  (mission history empty)\n");
        return;
    }
    for (int i = lm->historyTop; i >= 0; i--) {
        MissionRecord *m = &lm->historyArr[i];
        const char *tag = (m->status == MISSION_CANCELLED) ? "cancelled" : "dispatched";
        printf("  [%s] id=%d dest=%s\n", tag, m->missionId, m->destName);
    }
}
