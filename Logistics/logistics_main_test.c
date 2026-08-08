/* ============================================================
   logistics_main_test.c - Member 2's standalone sandbox
   ------------------------------------------------------------
   Exercises the Logistics module against a small Graph built
   with the real graph_api.h. NOT part of the team's main.c -
   compile this only for local testing, same convention as
   Member 1's main_test.c for the Graph module.
   ============================================================ */

#include <stdio.h>
#include "graph_api.h"
#include "logistics_api.h"

static void print_dispatched(const char *label, MissionRecord *m) {
    printf("%s: id=%d type=%d priority=%d dest=%s status=%d\n",
           label, m->missionId, m->type, m->priority, m->destName, m->status);
}

int main(void) {
    Graph *g = graph_create();

    int hq       = graph_add_location(g, "HQ Alpha", MILITARY_BASE);
    int border   = graph_add_location(g, "Border Post 7", BORDER_POST);
    int hospital = graph_add_location(g, "Field Hospital", MILITARY_HOSPITAL);
    int depot    = graph_add_location(g, "Supply Depot 3", SUPPLY_DEPOT);

    graph_add_link(g, hq, border, 5);
    graph_add_link(g, hq, hospital, 3);
    graph_add_link(g, hq, depot, 2);

    LogisticsManager *lm = logistics_init(g);

    printf("=== Creating missions ===\n");
    int m1 = mission_create(lm, MISSION_CONVOY, depot);
    int m2 = mission_create(lm, MISSION_MEDICAL_SUPPLY, hospital);
    int m3 = mission_create(lm, MISSION_EMERGENCY, border);
    printf("Created mission ids: %d %d %d\n", m1, m2, m3);

    /* Invalid destination should be rejected */
    int bad = mission_create(lm, MISSION_GENERAL, 999);
    printf("Invalid-destination create returned: %d (expect -1)\n\n", bad);

    printf("=== Before triage ===\n");
    mission_display_queue(lm);

    printf("\n=== Triage: assign priorities ===\n");
    mission_assign_priority(lm, m1, PRIORITY_LOW);
    mission_assign_priority(lm, m2, PRIORITY_HIGH);
    mission_assign_priority(lm, m3, PRIORITY_CRITICAL);

    printf("\n=== After triage (priority order) ===\n");
    mission_display_queue(lm);

    printf("\n=== Dispatch three missions in priority order ===\n");
    MissionRecord dispatched;
    while (mission_dispatch(lm, &dispatched)) {
        print_dispatched("Dispatched", &dispatched);
    }

    printf("\n=== Convoy slots after dispatch ===\n");
    mission_display_convoy_slots(lm);

    printf("\n=== History ===\n");
    mission_display_history(lm);

    printf("\n=== Undo last mission ===\n");
    mission_undo_last(lm);
    mission_display_queue(lm);
    mission_display_convoy_slots(lm);

    printf("\n=== Cancel a mission still pending, and one already dispatched ===\n");
    printf("cancel m2 (already dispatched, in history): %d\n", mission_cancel(lm, m2));
    printf("cancel m3 (already dispatched, in history): %d\n", mission_cancel(lm, m3));

    printf("\n=== Final state ===\n");
    mission_display_queue(lm);
    mission_display_history(lm);
    printf("pending count=%d history count=%d\n",
           logistics_pending_count(lm), logistics_history_count(lm));

    logistics_destroy(lm);
    graph_destroy(g);
    return 0;
}
