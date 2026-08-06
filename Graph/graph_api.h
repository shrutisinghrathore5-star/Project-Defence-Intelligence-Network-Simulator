#ifndef GRAPH_API_H
#define GRAPH_API_H

/* ============================================================
   PUBLIC API - Graph Engine (Member 1 / Team Leader)
   ------------------------------------------------------------
   Members 2, 3, 4: include ONLY this header. Never include
   graph_internal.h - it exists so DFS/BFS/MST/failure-sim code
   can share the real struct layout, but nobody outside this
   module should touch those fields directly. `Graph` here is
   an OPAQUE type (an incomplete struct): you can hold a
   `Graph *`, pass it around, and call these functions on it,
   but the compiler will stop you from reading/writing its
   internals - so a bug in your module can't corrupt the graph's
   linked lists.
   ============================================================ */

#define GRAPH_MAX_NODES     100
#define GRAPH_MAX_NAME_LEN  50

typedef enum {
    MILITARY_BASE,
    BORDER_POST,
    RADAR_STATION,
    AIRPORT,
    MILITARY_HOSPITAL,
    SUPPLY_DEPOT
} LocationType;

typedef struct Graph Graph;   /* opaque handle - fields are private */

/* Safe read-only snapshot of one node, returned BY VALUE into a
   struct you own - no pointers into the graph's internals. */
typedef struct {
    int id;
    char name[GRAPH_MAX_NAME_LEN];
    LocationType type;
    int active;
} LocationInfo;

/* ---------------- lifecycle ---------------- */
Graph *graph_create(void);
void   graph_destroy(Graph *g);

/* ---------------- core CRUD on the network ---------------- */
int  graph_add_location(Graph *g, const char *name, LocationType type); /* returns new id, -1 on failure */
int  graph_remove_location(Graph *g, int id);                           /* returns 1 on success */
int  graph_add_link(Graph *g, int srcId, int destId, int weight);
int  graph_remove_link(Graph *g, int srcId, int destId);
void graph_display(Graph *g);

/* ---------------- traversal ---------------- */
void graph_bfs(Graph *g, int startId);
void graph_dfs_recursive(Graph *g, int startId);
void graph_dfs_iterative(Graph *g, int startId);

/* ---------------- minimum spanning tree ---------------- */
void graph_kruskal_mst(Graph *g);
void graph_prim_mst(Graph *g);

/* ---------------- failure simulation ---------------- */
int  graph_fail_link(Graph *g, int srcId, int destId);
int  graph_restore_link(Graph *g, int srcId, int destId);
int  graph_fail_node(Graph *g, int id);
int  graph_is_connected(Graph *g);
void graph_report_unreachable(Graph *g, int fromId);

/* ---------------- read-only queries for OTHER modules ----------------
   Logistics (Member 2), Intel DB (Member 3), and UI (Member 4) use
   these instead of reaching into the struct - e.g. Logistics needs
   to know a base is still active before queuing a convoy to it;
   Intel DB needs the name/type to store alongside an asset record. */
int  graph_node_count(Graph *g);
int  graph_node_exists(Graph *g, int id);
int  graph_node_active(Graph *g, int id);
int  graph_get_location_info(Graph *g, int id, LocationInfo *outInfo); /* returns 1 if found */
const char *graph_type_to_string(LocationType t);

#endif
