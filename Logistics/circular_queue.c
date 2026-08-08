/* ============================================================
   circular_queue.c - ring buffer of convoy slots
   ------------------------------------------------------------
   Only LOGISTICS_MAX_CONVOY_SLOTS convoys can be physically
   en route at once. This is the textbook circular-queue case:
   a fixed pool of slots that gets reused as convoys return,
   with the ring wrapping via modulo arithmetic rather than
   ever shifting the array.

   Slot value -1 means free; any other value is the missionId
   currently holding that slot.
   ============================================================ */

#include <stdio.h>
#include "logistics_internal.h"

int cq_available(LogisticsManager *lm) {
    return LOGISTICS_MAX_CONVOY_SLOTS - lm->convoyCount;
}

int cq_acquire(LogisticsManager *lm, int missionId) {
    if (lm->convoyCount == LOGISTICS_MAX_CONVOY_SLOTS) return 0; /* all slots busy */

    lm->convoySlotMissionId[lm->convoyRear] = missionId;
    lm->convoyRear = (lm->convoyRear + 1) % LOGISTICS_MAX_CONVOY_SLOTS;
    lm->convoyCount++;
    return 1;
}

/* A convoy can return out of order relative to when it left, so
   release searches the ring for the owning slot rather than
   assuming strict FIFO return order. */
int cq_release(LogisticsManager *lm, int missionId) {
    if (lm->convoyCount == 0) return 0;

    for (int i = 0, idx = lm->convoyFront; i < LOGISTICS_MAX_CONVOY_SLOTS; i++, idx = (idx + 1) % LOGISTICS_MAX_CONVOY_SLOTS) {
        if (lm->convoySlotMissionId[idx] == missionId) {
            lm->convoySlotMissionId[idx] = -1;
            lm->convoyCount--;
            /* Advance front past any now-free leading slots so the
               ring stays tidy for display purposes. */
            while (lm->convoyCount > 0 && lm->convoySlotMissionId[lm->convoyFront] == -1) {
                lm->convoyFront = (lm->convoyFront + 1) % LOGISTICS_MAX_CONVOY_SLOTS;
            }
            if (lm->convoyCount == 0) {
                lm->convoyFront = lm->convoyRear;
            }
            return 1;
        }
    }
    return 0;
}

void cq_display(LogisticsManager *lm) {
    printf("  Convoy slots: %d/%d in use\n", lm->convoyCount, LOGISTICS_MAX_CONVOY_SLOTS);
    for (int i = 0; i < LOGISTICS_MAX_CONVOY_SLOTS; i++) {
        if (lm->convoySlotMissionId[i] != -1) {
            printf("    slot %d -> mission %d\n", i, lm->convoySlotMissionId[i]);
        }
    }
}
