#ifndef LOGISTICS_INTERNAL_H
#define LOGISTICS_INTERNAL_H

/* ============================================================
   PRIVATE - Logistics Module internals
   ------------------------------------------------------------
   Only queue.c, circular_queue.c, priority_queue.c, stack.c
   and mission.c may include this file. It exposes the real
   layout of `struct LogisticsManager` (opaque outside this
   module) and the low-level primitives each data-structure
   file implements over it.
   ============================================================ */

#include "graph_api.h"
#include "logistics_api.h"

struct LogisticsManager {
    Graph *graph;   /* borrowed reference - this module never frees it */

    /* --- intake FIFO: plain array-backed queue --- */
    MissionRecord intakeArr[LOGISTICS_MAX_PENDING];
    int intakeFront;
    int intakeRear;
    int intakeCount;

    /* --- priority queue: binary max-heap, array-backed --- */
    MissionRecord heapArr[LOGISTICS_MAX_PQ];
    int heapSize;

    /* --- circular queue: convoy slots (ring buffer of slot owners) --- */
    int convoySlotMissionId[LOGISTICS_MAX_CONVOY_SLOTS]; /* -1 = free */
    int convoyFront;
    int convoyRear;
    int convoyCount;

    /* --- history stack: array-backed LIFO --- */
    MissionRecord historyArr[LOGISTICS_MAX_HISTORY];
    int historyTop;   /* -1 = empty */

    int nextMissionId;
    int nextSequenceNum;
};

/* ---------------- queue.c: generic FIFO intake primitives ---------------- */
int  fifo_push(LogisticsManager *lm, MissionRecord m);
int  fifo_pop_by_id(LogisticsManager *lm, int missionId, MissionRecord *out);
int  fifo_is_empty(LogisticsManager *lm);
int  fifo_is_full(LogisticsManager *lm);
void fifo_display(LogisticsManager *lm);

/* ---------------- circular_queue.c: convoy slot ring buffer ---------------- */
int  cq_acquire(LogisticsManager *lm, int missionId);
int  cq_release(LogisticsManager *lm, int missionId);
int  cq_available(LogisticsManager *lm);
void cq_display(LogisticsManager *lm);

/* ---------------- priority_queue.c: binary max-heap ---------------- */
int  heap_push(LogisticsManager *lm, MissionRecord m);
int  heap_pop_max(LogisticsManager *lm, MissionRecord *out);
int  heap_is_empty(LogisticsManager *lm);
int  heap_is_full(LogisticsManager *lm);
int  heap_cancel_by_id(LogisticsManager *lm, int missionId); /* lazy-delete flag flip */
void heap_display(LogisticsManager *lm);

/* ---------------- stack.c: mission history LIFO ---------------- */
int  stack_push(LogisticsManager *lm, MissionRecord m);
int  stack_pop(LogisticsManager *lm, MissionRecord *out);
int  stack_is_empty(LogisticsManager *lm);
int  stack_is_full(LogisticsManager *lm);
void stack_display(LogisticsManager *lm);

#endif /* LOGISTICS_INTERNAL_H */
