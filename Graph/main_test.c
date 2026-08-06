#include <stdio.h>
#include "graph_api.h"

/* This is a stand-in for how Member 4's menu-driven UI (and Members
   2/3's modules) will actually talk to the Graph engine: only
   graph_api.h is included, and Graph is used purely as an opaque
   handle returned by graph_create().

   Build:
     gcc graph.c bfs.c dfs.c mst.c network_failure.c main_test.c -o sentinel_graph_test
*/
int main(void) {
    Graph *g = graph_create();
    if (!g) { printf("Failed to allocate graph.\n"); return 1; }

    int hq      = graph_add_location(g, "HQ_Delta",        MILITARY_BASE);
    int border1 = graph_add_location(g, "BorderPost_Alpha", BORDER_POST);
    int radar1  = graph_add_location(g, "Radar_North",      RADAR_STATION);
    int airport = graph_add_location(g, "Airbase_Falcon",   AIRPORT);
    int depot   = graph_add_location(g, "SupplyDepot_1",    SUPPLY_DEPOT);
    int hosp    = graph_add_location(g, "FieldHospital_1",  MILITARY_HOSPITAL);

    graph_add_link(g, hq, border1, 12);
    graph_add_link(g, hq, radar1, 8);
    graph_add_link(g, hq, airport, 20);
    graph_add_link(g, radar1, border1, 5);
    graph_add_link(g, airport, depot, 7);
    graph_add_link(g, depot, hosp, 4);
    graph_add_link(g, hq, depot, 15);

    graph_display(g);

    graph_bfs(g, hq);
    graph_dfs_recursive(g, hq);
    graph_dfs_iterative(g, hq);

    graph_kruskal_mst(g);
    graph_prim_mst(g);

    printf("\nNetwork connected? %s\n", graph_is_connected(g) ? "YES" : "NO");

    /* --- example of Member 2 / Member 3 style read-only queries --- */
    LocationInfo info;
    if (graph_get_location_info(g, depot, &info)) {
        printf("\n[Logistics check] Node %d = %s (%s), active=%d\n",
               info.id, info.name, graph_type_to_string(info.type), info.active);
    }

    /* simulate a failure */
    graph_fail_link(g, hq, radar1);
    graph_fail_link(g, radar1, border1);
    graph_report_unreachable(g, hq);
    printf("Network connected after failures? %s\n",
           graph_is_connected(g) ? "YES" : "NO");

    graph_fail_node(g, airport);
    printf("Network connected after node loss? %s\n",
           graph_is_connected(g) ? "YES" : "NO");

    graph_remove_location(g, hosp);
    graph_display(g);

    graph_destroy(g);
    return 0;
}
