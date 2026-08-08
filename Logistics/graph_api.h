#ifndef GRAPH_API_H
#define GRAPH_API_H

/*
 * ============================================================
 * GRAPH PUBLIC API
 * ============================================================
 *
 * Other modules such as:
 *   - Logistics (Member 2)
 *   - Intel DB (Member 3)
 *   - UI (Member 4)
 *
 * should include ONLY this file.
 *
 * They must NOT access the internal Graph structure directly.
 *
 * ============================================================
 */

#define GRAPH_MAX_NODES     100
#define GRAPH_MAX_NAME_LEN  50


/* ============================================================
 * LOCATION TYPES
 * ============================================================
 */

typedef enum
{
    MILITARY_BASE,
    BORDER_POST,
    RADAR_STATION,
    AIRPORT,
    MILITARY_HOSPITAL,
    SUPPLY_DEPOT

} LocationType;


/* ============================================================
 * OPAQUE GRAPH TYPE
 * ============================================================
 *
 * The actual structure of Graph is private to graph.c.
 *
 */

typedef struct Graph Graph;


/* ============================================================
 * LOCATION INFORMATION
 * ============================================================
 *
 * Safe copy of information about one location.
 *
 */

typedef struct
{
    int id;

    char name[GRAPH_MAX_NAME_LEN];

    LocationType type;

    int active;

} LocationInfo;


/* ============================================================
 * GRAPH LIFECYCLE
 * ============================================================
 */

Graph *graph_create(void);

void graph_destroy(Graph *g);


/* ============================================================
 * LOCATION CRUD
 * ============================================================
 */

/*
 * Add a new location.
 *
 * Returns:
 *   New location ID on success
 *   -1 on failure
 */
int graph_add_location(
    Graph *g,
    const char *name,
    LocationType type
);


/*
 * Remove/deactivate a location.
 *
 * Returns:
 *   1 = success
 *   0 = failure
 */
int graph_remove_location(
    Graph *g,
    int id
);


/* ============================================================
 * LINK CRUD
 * ============================================================
 */

/*
 * Add a connection between two locations.
 *
 * weight = distance/cost of connection.
 *
 * Returns:
 *   1 = success
 *   0 = failure
 */
int graph_add_link(
    Graph *g,
    int srcId,
    int destId,
    int weight
);


/*
 * Remove a connection.
 *
 * Returns:
 *   1 = success
 *   0 = failure
 */
int graph_remove_link(
    Graph *g,
    int srcId,
    int destId
);


/*
 * Display complete graph.
 */
void graph_display(Graph *g);


/* ============================================================
 * GRAPH TRAVERSAL
 * ============================================================
 */

/*
 * Breadth First Search
 */
void graph_bfs(
    Graph *g,
    int startId
);


/*
 * Recursive Depth First Search
 */
void graph_dfs_recursive(
    Graph *g,
    int startId
);


/*
 * Iterative Depth First Search
 */
void graph_dfs_iterative(
    Graph *g,
    int startId
);


/* ============================================================
 * MINIMUM SPANNING TREE
 * ============================================================
 */

/*
 * Kruskal's Minimum Spanning Tree
 */
void graph_kruskal_mst(
    Graph *g
);


/*
 * Prim's Minimum Spanning Tree
 */
void graph_prim_mst(
    Graph *g
);


/* ============================================================
 * FAILURE SIMULATION
 * ============================================================
 */

/*
 * Temporarily fail a link.
 */
int graph_fail_link(
    Graph *g,
    int srcId,
    int destId
);


/*
 * Restore a failed link.
 */
int graph_restore_link(
    Graph *g,
    int srcId,
    int destId
);


/*
 * Fail/deactivate a node.
 */
int graph_fail_node(
    Graph *g,
    int id
);


/*
 * Check whether active graph is connected.
 *
 * Returns:
 *   1 = connected
 *   0 = not connected
 */
int graph_is_connected(
    Graph *g
);


/*
 * Display locations that cannot be reached
 * from the specified location.
 */
void graph_report_unreachable(
    Graph *g,
    int fromId
);


/* ============================================================
 * READ-ONLY QUERIES
 * ============================================================
 *
 * These functions are particularly important for
 * Member 2 (Logistics).
 *
 * Logistics can check whether a destination exists
 * and whether it is currently active without accessing
 * Graph's internal structure.
 *
 */


/*
 * Get total number of locations.
 */
int graph_node_count(
    Graph *g
);


/*
 * Check whether a location ID exists.
 *
 * Returns:
 *   1 = exists
 *   0 = does not exist
 */
int graph_node_exists(
    Graph *g,
    int id
);


/*
 * Check whether a location is active.
 *
 * Returns:
 *   1 = active
 *   0 = inactive/not found
 */
int graph_node_active(
    Graph *g,
    int id
);


/*
 * Get safe information about a location.
 *
 * Information is copied into outInfo.
 *
 * Returns:
 *   1 = location found
 *   0 = location not found/error
 */
int graph_get_location_info(
    Graph *g,
    int id,
    LocationInfo *outInfo
);


/*
 * Convert LocationType to readable text.
 */
const char *graph_type_to_string(
    LocationType t
);


#endif
