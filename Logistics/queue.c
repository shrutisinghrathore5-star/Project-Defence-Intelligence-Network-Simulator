/* ============================================================
   queue.c - generic array-backed FIFO intake queue
   ------------------------------------------------------------
   Newly created missions land here first, in the order they
   were reported, before triage assigns a priority and moves
   them into the priority queue (priority_queue.c).

   This file knows nothing about Graph or priorities - it only
   moves MissionRecord values in and out of a fixed array.
   ============================================================ */

#include <stdio.h>
#include "logistics_internal.h"

int fifo_is_empty(LogisticsManager *lm) {
    return lm->intakeCount == 0;
}

int fifo_is_full(LogisticsManager *lm) {
    return lm->intakeCount == LOGISTICS_MAX_PENDING;
}

int fifo_push(LogisticsManager *lm, MissionRecord m) {
    if (fifo_is_full(lm)) return 0;

    lm->intakeArr[lm->intakeRear] = m;
    lm->intakeRear = (lm->intakeRear + 1) % LOGISTICS_MAX_PENDING;
    lm->intakeCount++;
    return 1;
}

/* Intake is triaged FIFO in the common case, but a specific
   mission (by id) can also be pulled out of the middle when
   mission_assign_priority() targets a particular mission rather
   than "whatever is at the front". We do this with a linear
   scan + compaction shift, which is fine at this queue's scale
   (<= LOGISTICS_MAX_PENDING) and keeps front/rear bookkeeping
   simple for the true FIFO case. */
int fifo_pop_by_id(LogisticsManager *lm, int missionId, MissionRecord *out) {
    if (fifo_is_empty(lm)) return 0;

    for (int i = 0, idx = lm->intakeFront; i < lm->intakeCount; i++, idx = (idx + 1) % LOGISTICS_MAX_PENDING) {
        if (lm->intakeArr[idx].missionId == missionId) {
            *out = lm->intakeArr[idx];

            /* Shift everything after idx back by one slot (logical, wrap-aware) */
            int shiftCount = lm->intakeCount - i - 1;
            int from = (idx + 1) % LOGISTICS_MAX_PENDING;
            int to = idx;
            for (int s = 0; s < shiftCount; s++) {
                lm->intakeArr[to] = lm->intakeArr[from];
                to = (to + 1) % LOGISTICS_MAX_PENDING;
                from = (from + 1) % LOGISTICS_MAX_PENDING;
            }
            lm->intakeRear = (lm->intakeRear - 1 + LOGISTICS_MAX_PENDING) % LOGISTICS_MAX_PENDING;
            lm->intakeCount--;
            return 1;
        }
    }
    return 0;
}

void fifo_display(LogisticsManager *lm) {
    if (fifo_is_empty(lm)) {
        printf("  (intake queue empty)\n");
        return;
    }
    for (int i = 0, idx = lm->intakeFront; i < lm->intakeCount; i++, idx = (idx + 1) % LOGISTICS_MAX_PENDING) {
        MissionRecord *m = &lm->intakeArr[idx];
        printf("  [awaiting triage] id=%d dest=%s\n", m->missionId, m->destName);
    }
}
