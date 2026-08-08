/* ============================================================
   mission.c - lifecycle glue for the Logistics module
   ------------------------------------------------------------
   Implements every function declared in logistics_api.h by
   orchestrating the four structures in queue.c / circular_queue.c
   / priority_queue.c / stack.c as a MissionRecord moves through:

       create -> intake FIFO -> assign priority -> priority heap
              -> dispatch (+ convoy slot if MISSION_CONVOY)
              -> history stack -> (optional) undo -> back to heap

   This is the ONLY file in the Logistics module that includes
   or calls graph_api.h. If Member 1 changes Graph internals,
   this is the one file to check.
   ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logistics_internal.h"



/* ---------------- lifecycle ---------------- */

LogisticsManager *logistics_init(Graph *g) {
    LogisticsManager *lm = (LogisticsManager *)malloc(sizeof(LogisticsManager));
    if (!lm) return NULL;

    lm->graph = g;   /* borrowed - this module never frees it */

    lm->intakeFront = 0;
    lm->intakeRear = 0;
    lm->intakeCount = 0;

    lm->heapSize = 0;

    for (int i = 0; i < LOGISTICS_MAX_CONVOY_SLOTS; i++) {
        lm->convoySlotMissionId[i] = -1;
    }
    lm->convoyFront = 0;
    lm->convoyRear = 0;
    lm->convoyCount = 0;

    lm->historyTop = -1;

    lm->nextMissionId = 1;
    lm->nextSequenceNum = 1;

    return lm;
}

void logistics_destroy(LogisticsManager *lm) {
    if (!lm) return;
    free(lm);   /* everything else lives in fixed arrays inside the struct */
}

/* ---------------- internal search helpers ---------------- */

/* Looks for missionId across intake, heap, and history so
   mission_get_info()/mission_cancel() have one place to look
   regardless of which stage the mission is currently in.
   Returns a pointer to the live record (NOT a copy) so callers
   in this file can mutate status in place; never expose this
   pointer outside mission.c. */
static MissionRecord *find_live_record(LogisticsManager *lm, int missionId) {
    for (int i = 0, idx = lm->intakeFront; i < lm->intakeCount; i++, idx = (idx + 1) % LOGISTICS_MAX_PENDING) {
        if (lm->intakeArr[idx].missionId == missionId) return &lm->intakeArr[idx];
    }
    for (int i = 0; i < lm->heapSize; i++) {
        if (lm->heapArr[i].missionId == missionId) return &lm->heapArr[i];
    }
    for (int i = 0; i <= lm->historyTop; i++) {
        if (lm->historyArr[i].missionId == missionId) return &lm->historyArr[i];
    }
    return NULL;
}

/* ---------------- mission creation & triage ---------------- */

int mission_create(LogisticsManager *lm, MissionType type, int destId) {
    if (!lm) return -1;

    /* Only place in the whole Logistics module that talks to Graph. */
    if (!graph_node_exists(lm->graph, destId) || !graph_node_active(lm->graph, destId)) {
        return -1;   /* invalid or offline destination */
    }

    LocationInfo info;
    if (!graph_get_location_info(lm->graph, destId, &info)) {
        return -1;
    }

    MissionRecord m;
    m.missionId = lm->nextMissionId;
    m.type = type;
    m.priority = PRIORITY_LOW;   /* default until triage assigns one */
    m.destId = destId;
    strncpy(m.destName, info.name, GRAPH_MAX_NAME_LEN - 1);
    m.destName[GRAPH_MAX_NAME_LEN - 1] = '\0';
    m.status = MISSION_PENDING;
    m.sequenceNum = lm->nextSequenceNum;

    if (!fifo_push(lm, m)) {
        return -1;   /* intake queue full */
    }

    lm->nextMissionId++;
    lm->nextSequenceNum++;
    return m.missionId;
}

int mission_assign_priority(LogisticsManager *lm, int missionId, MissionPriority p) {
    if (!lm) return 0;

    MissionRecord m;
    if (!fifo_pop_by_id(lm, missionId, &m)) return 0;

    m.priority = p;
    if (!mission_enqueue(lm, m)) {
        /* Priority heap full - put the mission back at intake so it
           isn't silently lost, and report failure to the caller. */
        fifo_push(lm, m);
        return 0;
    }
    return 1;
}

/* ---------------- priority queue (dispatch ordering) ---------------- */

int mission_enqueue(LogisticsManager *lm, MissionRecord m) {
    if (!lm) return 0;
    return heap_push(lm, m);
}

int mission_dispatch(LogisticsManager *lm, MissionRecord *outDispatched) {
    if (!lm) return 0;

    MissionRecord m;
    if (!heap_pop_max(lm, &m)) return 0;   /* nothing pending */

    if (m.type == MISSION_CONVOY) {
        if (!cq_acquire(lm, m.missionId)) {
            /* No convoy slot free - put it back and report failure.
               Re-pushing preserves its priority/sequenceNum, so it
               will be reconsidered first next time a slot opens up. */
            mission_enqueue(lm, m);
            return 0;
        }
    }

    /* Defensive re-check: the destination may have gone offline
       (graph_fail_node) after creation but before reaching the
       front of the queue. */
    if (!graph_node_active(lm->graph, m.destId)) {
        if (m.type == MISSION_CONVOY) cq_release(lm, m.missionId);
        mission_enqueue(lm, m);   /* leave pending, don't dispatch */
        return 0;
    }

    m.status = MISSION_DISPATCHED;
    stack_push(lm, m);

    if (outDispatched) *outDispatched = m;
    return 1;
}

/* ---------------- circular queue wrappers ---------------- */

int convoy_slot_acquire(LogisticsManager *lm, int missionId) {
    return cq_acquire(lm, missionId);
}

int convoy_slot_release(LogisticsManager *lm, int missionId) {
    return cq_release(lm, missionId);
}

int convoy_slots_available(LogisticsManager *lm) {
    return cq_available(lm);
}

/* ---------------- history stack / undo ---------------- */

int mission_history_push(LogisticsManager *lm, MissionRecord m) {
    return stack_push(lm, m);
}

int mission_history_pop(LogisticsManager *lm, MissionRecord *outUndone) {
    return stack_pop(lm, outUndone);
}

int mission_undo_last(LogisticsManager *lm) {
    if (!lm) return 0;

    MissionRecord m;
    if (!stack_pop(lm, &m)) return 0;   /* nothing to undo */

    if (m.type == MISSION_CONVOY) {
        cq_release(lm, m.missionId);
    }

    m.status = MISSION_PENDING;
    return mission_enqueue(lm, m);
}

/* ---------------- cancel & queries ---------------- */

int mission_cancel(LogisticsManager *lm, int missionId) {
    if (!lm) return 0;

    /* Try intake first (simple removal, not yet triaged). */
    MissionRecord m;
    if (fifo_pop_by_id(lm, missionId, &m)) {
        return 1;   /* removed outright - never entered priority queue */
    }

    /* Try the priority queue (lazy-delete flag). */
    if (heap_cancel_by_id(lm, missionId)) {
        return 1;
    }

    /* Already dispatched - find it in history, flip status, and
       release its convoy slot if it was holding one. */
    for (int i = 0; i <= lm->historyTop; i++) {
        if (lm->historyArr[i].missionId == missionId) {
            lm->historyArr[i].status = MISSION_CANCELLED;
            if (lm->historyArr[i].type == MISSION_CONVOY) {
                cq_release(lm, missionId);
            }
            return 1;
        }
    }

    return 0;   /* not found anywhere */
}

int mission_get_info(LogisticsManager *lm, int missionId, MissionRecord *out) {
    if (!lm || !out) return 0;

    MissionRecord *found = find_live_record(lm, missionId);
    if (!found) return 0;

    *out = *found;
    return 1;
}

void mission_display_queue(LogisticsManager *lm) {
    printf("Pending (awaiting triage):\n");
    fifo_display(lm);
    printf("Pending (priority order):\n");
    heap_display(lm);
}

void mission_display_history(LogisticsManager *lm) {
    printf("Mission history (most recent first):\n");
    stack_display(lm);
}

void mission_display_convoy_slots(LogisticsManager *lm) {
    cq_display(lm);
}

int logistics_pending_count(LogisticsManager *lm) {
    if (!lm) return 0;
    return lm->intakeCount + lm->heapSize;
}

int logistics_history_count(LogisticsManager *lm) {
    if (!lm) return 0;
    return lm->historyTop + 1;
}
