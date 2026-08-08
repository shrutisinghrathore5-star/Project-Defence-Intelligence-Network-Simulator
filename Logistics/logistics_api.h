#ifndef LOGISTICS_API_H
#define LOGISTICS_API_H

#include "graph_api.h"

/*
 * PUBLIC LOGISTICS API
 *
 * Members outside the Logistics module should include ONLY this file.
 *
 * Do NOT include:
 *     logistics_internal.h
 *
 * LogisticsManager is an opaque type. External modules can hold a
 * LogisticsManager* and call the functions below, but cannot access
 * its internal queues, priority queue, stack, or other fields.
 */

/* ============================================================
   Configuration
   ============================================================ */

#define LOGISTICS_MAX_PENDING       100
#define LOGISTICS_MAX_PQ            100
#define LOGISTICS_MAX_HISTORY       100
#define LOGISTICS_MAX_CONVOY_SLOTS    8


/* ============================================================
   Mission Types
   ============================================================ */

typedef enum
{
    MISSION_CONVOY,
    MISSION_MEDICAL_SUPPLY,
    MISSION_WEAPON_SUPPLY,
    MISSION_EMERGENCY,
    MISSION_GENERAL

} MissionType;


/* ============================================================
   Mission Priority
   ============================================================ */

typedef enum
{
    PRIORITY_LOW      = 1,
    PRIORITY_MEDIUM   = 2,
    PRIORITY_HIGH     = 3,
    PRIORITY_CRITICAL = 4

} MissionPriority;


/* ============================================================
   Mission Status
   ============================================================ */

typedef enum
{
    MISSION_PENDING,
    MISSION_DISPATCHED,
    MISSION_CANCELLED,
    MISSION_COMPLETED

} MissionStatus;


/* ============================================================
   Public Mission Snapshot
   ============================================================ */

/*
 * Safe read-only snapshot.
 *
 * This structure contains copies of mission information.
 * It does not expose internal queue, priority queue, stack,
 * or heap pointers.
 */

typedef struct
{
    int             missionId;
    MissionType     type;
    MissionPriority priority;

    int             destId;

    char            destName[GRAPH_MAX_NAME_LEN];

    MissionStatus   status;

    /*
     * Creation order.
     * Used to resolve equal-priority missions:
     * earlier mission -> higher dispatch preference.
     */
    int sequenceNum;

} MissionRecord;


/* ============================================================
   Opaque Logistics Manager
   ============================================================ */

typedef struct LogisticsManager LogisticsManager;


/* ============================================================
   Lifecycle
   ============================================================ */

/*
 * Creates and initializes the Logistics Manager.
 *
 * The Graph pointer is shared with the Graph module.
 * LogisticsManager does NOT own the Graph and will NOT free it.
 *
 * Returns:
 *     valid LogisticsManager* on success
 *     NULL on allocation failure
 */
LogisticsManager *logistics_init(Graph *g);


/*
 * Frees all Logistics module memory.
 *
 * Does NOT destroy or free the shared Graph.
 */
void logistics_destroy(LogisticsManager *lm);


/* ============================================================
   Mission Creation / Intake Queue
   ============================================================ */

/*
 * Creates a new mission and places it into the FIFO intake queue.
 *
 * Destination is validated using:
 *     graph_node_exists()
 *     graph_node_active()
 *
 * Destination name is obtained using:
 *     graph_get_location_info()
 *
 * Returns:
 *     new mission ID
 *     -1 if destination is invalid/inactive
 *     -1 if intake queue is full
 */
int mission_create(
    LogisticsManager *lm,
    MissionType type,
    int destId
);


/* ============================================================
   Triage / Priority Queue
   ============================================================ */

/*
 * Removes the specified mission from the intake queue,
 * assigns its priority, and inserts it into the priority queue.
 *
 * Returns:
 *     1 on success
 *     0 on failure
 */
int mission_assign_priority(
    LogisticsManager *lm,
    int missionId,
    MissionPriority priority
);


/* ============================================================
   Dispatch
   ============================================================ */

/*
 * Dispatches the highest-priority pending mission.
 *
 * Priority order:
 *
 *     CRITICAL > HIGH > MEDIUM > LOW
 *
 * For equal priorities, earlier sequenceNum is preferred.
 *
 * If the mission is a convoy, a convoy slot must be available.
 *
 * On successful dispatch:
 *     status -> MISSION_DISPATCHED
 *     mission is added to history
 *     mission information is copied into outDispatched
 *
 * Returns:
 *     1 on successful dispatch
 *     0 if no mission can be dispatched
 */
int mission_dispatch(
    LogisticsManager *lm,
    MissionRecord *outDispatched
);


/* ============================================================
   Convoy Slot Management
   ============================================================ */

/*
 * Manually acquires a convoy slot for a mission.
 *
 * Returns:
 *     1 if slot acquired
 *     0 if no slot is available or operation fails
 */
int convoy_slot_acquire(
    LogisticsManager *lm,
    int missionId
);


/*
 * Releases the convoy slot associated with a mission.
 *
 * Returns:
 *     1 if released
 *     0 if mission/slot was not found
 */
int convoy_slot_release(
    LogisticsManager *lm,
    int missionId
);


/*
 * Returns the number of currently free convoy slots.
 */
int convoy_slots_available(
    LogisticsManager *lm
);


/* ============================================================
   Undo
   ============================================================ */

/*
 * Undoes the most recently dispatched mission.
 *
 * Behaviour:
 *     1. Remove mission from history stack
 *     2. Release convoy slot if applicable
 *     3. Change status to MISSION_PENDING
 *     4. Reinsert mission into priority queue
 *
 * Returns:
 *     1 on success
 *     0 if history is empty or operation fails
 */
int mission_undo_last(
    LogisticsManager *lm
);


/* ============================================================
   Cancellation
   ============================================================ */

/*
 * Cancels a mission.
 *
 * The mission may currently be:
 *     - in intake queue
 *     - in priority queue
 *     - already dispatched
 *
 * If a dispatched convoy is cancelled, its convoy slot is released.
 *
 * The implementation may use lazy deletion internally.
 *
 * Returns:
 *     1 on success
 *     0 if mission was not found
 */
int mission_cancel(
    LogisticsManager *lm,
    int missionId
);


/* ============================================================
   Mission Queries
   ============================================================ */

/*
 * Retrieves a safe copy of a mission record.
 *
 * The caller receives a copy and cannot modify LogisticsManager
 * internals through it.
 *
 * Returns:
 *     1 if mission exists
 *     0 if not found
 */
int mission_get_info(
    LogisticsManager *lm,
    int missionId,
    MissionRecord *out
);


/*
 * Number of currently pending missions.
 */
int logistics_pending_count(
    LogisticsManager *lm
);


/*
 * Number of missions currently stored in history.
 */
int logistics_history_count(
    LogisticsManager *lm
);


/* ============================================================
   Display / Reporting
   ============================================================ */

/*
 * Displays missions waiting for triage and missions
 * currently present in the priority queue.
 */
void mission_display_queue(
    LogisticsManager *lm
);


/*
 * Displays mission history from most recent to oldest.
 */
void mission_display_history(
    LogisticsManager *lm
);


/*
 * Displays current convoy-slot usage.
 */
void mission_display_convoy_slots(
    LogisticsManager *lm
);


#endif /* LOGISTICS_API_H */
