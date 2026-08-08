/* ============================================================
   priority_queue.c - binary max-heap, array-backed
   ------------------------------------------------------------
   Drives dispatch order: the mission at the front is always
   the most urgent one currently triaged. Ordering key is
   (priority DESC, sequenceNum ASC) - equal priorities dispatch
   in creation order, so a burst of HIGH-priority missions can't
   starve an older MEDIUM one indefinitely once things settle.

   Cancelled entries are lazy-deleted (status flipped, not
   removed) and skipped by heap_pop_max/heap_display so the
   sift-up/sift-down logic stays a plain textbook heap.
   ============================================================ */

#include <stdio.h>
#include "logistics_internal.h"

/* Returns 1 if record at index a should sit above index b in the heap. */
static int higher_priority(MissionRecord *a, MissionRecord *b) {
    if (a->priority != b->priority) return a->priority > b->priority;
    return a->sequenceNum < b->sequenceNum;   /* earlier created wins ties */
}

static void swap(MissionRecord *a, MissionRecord *b) {
    MissionRecord tmp = *a;
    *a = *b;
    *b = tmp;
}

static void sift_up(LogisticsManager *lm, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (higher_priority(&lm->heapArr[i], &lm->heapArr[parent])) {
            swap(&lm->heapArr[i], &lm->heapArr[parent]);
            i = parent;
        } else {
            break;
        }
    }
}

static void sift_down(LogisticsManager *lm, int i) {
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int best = i;

        if (left < lm->heapSize && higher_priority(&lm->heapArr[left], &lm->heapArr[best])) best = left;
        if (right < lm->heapSize && higher_priority(&lm->heapArr[right], &lm->heapArr[best])) best = right;

        if (best == i) break;
        swap(&lm->heapArr[i], &lm->heapArr[best]);
        i = best;
    }
}

int heap_is_empty(LogisticsManager *lm) {
    return lm->heapSize == 0;
}

int heap_is_full(LogisticsManager *lm) {
    return lm->heapSize == LOGISTICS_MAX_PQ;
}

int heap_push(LogisticsManager *lm, MissionRecord m) {
    if (heap_is_full(lm)) return 0;

    lm->heapArr[lm->heapSize] = m;
    sift_up(lm, lm->heapSize);
    lm->heapSize++;
    return 1;
}

/* Skips (and physically compacts out) any CANCELLED entries it
   encounters at the root, so callers never receive a cancelled
   mission from dispatch. */
int heap_pop_max(LogisticsManager *lm, MissionRecord *out) {
    while (lm->heapSize > 0) {
        MissionRecord top = lm->heapArr[0];

        lm->heapSize--;
        lm->heapArr[0] = lm->heapArr[lm->heapSize];
        sift_down(lm, 0);

        if (top.status == MISSION_CANCELLED) continue; /* skip, keep popping */

        *out = top;
        return 1;
    }
    return 0;
}

/* Lazy-delete: flips the status flag in place. heap_pop_max and
   heap_display both skip CANCELLED entries, so no restructuring
   of the heap is needed here. */
int heap_cancel_by_id(LogisticsManager *lm, int missionId) {
    for (int i = 0; i < lm->heapSize; i++) {
        if (lm->heapArr[i].missionId == missionId) {
            lm->heapArr[i].status = MISSION_CANCELLED;
            return 1;
        }
    }
    return 0;
}

void heap_display(LogisticsManager *lm) {
    int shown = 0;
    for (int i = 0; i < lm->heapSize; i++) {
        if (lm->heapArr[i].status == MISSION_CANCELLED) continue;
        printf("  [priority %d] id=%d dest=%s\n",
               lm->heapArr[i].priority, lm->heapArr[i].missionId, lm->heapArr[i].destName);
        shown++;
    }
    if (!shown) printf("  (priority queue empty)\n");
}
