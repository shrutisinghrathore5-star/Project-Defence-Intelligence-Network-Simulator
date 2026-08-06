# Project Sentinel — Graph Module Integration Contract
**Owner:** Shruti Singh (Team Leader) | **File to include:** `graph_api.h` only

This document is the interface contract between the Graph Engine and the
other three modules. If you're on Logistics, Intel DB, or UI, this is
everything you need — you never touch `graph.c`, `bfs.c`, `dfs.c`,
`mst.c`, `network_failure.c`, or `graph_internal.h` directly.

---

## 1. Why it's built this way

`Graph` is an **opaque type** — declared in `graph_api.h` as
`typedef struct Graph Graph;` with no visible fields. You can hold a
`Graph *` and pass it to any function below, but the compiler will
refuse `g->something` outside the Graph module's own `.c` files
(verified: attempting it throws `invalid use of incomplete typedef`).
This means a bug in Logistics or the UI can never corrupt the
network's internal linked lists — you can only change the graph
through the functions we've published, which validate their inputs.

## 2. Getting a handle

```c
#include "graph_api.h"

Graph *g = graph_create();   // call once, at program start
...
graph_destroy(g);            // call once, at program exit
```

Pass this same `Graph *` into every other module's `init`/`setup`
function (Logistics, Intel DB) so all four modules operate on one
shared network.

## 3. Types you'll use

```c
typedef enum {
    MILITARY_BASE, BORDER_POST, RADAR_STATION,
    AIRPORT, MILITARY_HOSPITAL, SUPPLY_DEPOT
} LocationType;

typedef struct {
    int id;
    char name[GRAPH_MAX_NAME_LEN];
    LocationType type;
    int active;
} LocationInfo;
```

`LocationInfo` is a **copy**, not a pointer into the graph — safe to
store, print, or pass around freely.

## 4. Full function reference

| Function | Signature | Returns | Notes |
|---|---|---|---|
| `graph_create` | `Graph *graph_create(void)` | new handle | call once |
| `graph_destroy` | `void graph_destroy(Graph *g)` | — | frees all memory |
| `graph_add_location` | `int graph_add_location(Graph *g, const char *name, LocationType type)` | new node id, or `-1` | |
| `graph_remove_location` | `int graph_remove_location(Graph *g, int id)` | `1`=ok, `0`=fail | also removes touching edges |
| `graph_add_link` | `int graph_add_link(Graph *g, int srcId, int destId, int weight)` | `1`=ok, `0`=fail | undirected |
| `graph_remove_link` | `int graph_remove_link(Graph *g, int srcId, int destId)` | `1`=ok, `0`=fail | |
| `graph_display` | `void graph_display(Graph *g)` | — | prints full topology |
| `graph_bfs` | `void graph_bfs(Graph *g, int startId)` | — | prints traversal order |
| `graph_dfs_recursive` | `void graph_dfs_recursive(Graph *g, int startId)` | — | |
| `graph_dfs_iterative` | `void graph_dfs_iterative(Graph *g, int startId)` | — | |
| `graph_kruskal_mst` | `void graph_kruskal_mst(Graph *g)` | — | prints MST + total cost |
| `graph_prim_mst` | `void graph_prim_mst(Graph *g)` | — | |
| `graph_fail_link` | `int graph_fail_link(Graph *g, int srcId, int destId)` | `1`=found, `0`=not found | marks link down, doesn't delete it |
| `graph_restore_link` | `int graph_restore_link(Graph *g, int srcId, int destId)` | `1`/`0` | |
| `graph_fail_node` | `int graph_fail_node(Graph *g, int id)` | `1`/`0` | marks node offline |
| `graph_is_connected` | `int graph_is_connected(Graph *g)` | `1`=connected, `0`=not | checks only *active* nodes/links |
| `graph_report_unreachable` | `void graph_report_unreachable(Graph *g, int fromId)` | — | prints list |
| `graph_node_count` | `int graph_node_count(Graph *g)` | count of existing nodes | |
| `graph_node_exists` | `int graph_node_exists(Graph *g, int id)` | `1`/`0` | validate ids before use |
| `graph_node_active` | `int graph_node_active(Graph *g, int id)` | `1`/`0` | |
| `graph_get_location_info` | `int graph_get_location_info(Graph *g, int id, LocationInfo *out)` | `1`=filled, `0`=not found | **your main lookup function** |
| `graph_type_to_string` | `const char *graph_type_to_string(LocationType t)` | string | for display |

---

## 5. How each module is expected to use it

### Shubhi Saxena (Member 2) — Logistics (Queue, Priority Queue, Stack)
Before queuing a convoy or mission to a destination, check the target
is real and reachable:

```c
if (!graph_node_exists(g, destId) || !graph_node_active(g, destId)) {
    printf("Cannot route convoy: destination unreachable.\n");
} else {
    LocationInfo info;
    graph_get_location_info(g, destId, &info);
    // enqueue mission with info.name for display, etc.
}
```
For emergency route planning, you can call `graph_is_connected(g)` or
`graph_report_unreachable(g, hqId)` after a failure event to decide
whether a mission needs rerouting.

### Shiwani (Member 3) — Intel DB (Hashing, BST, Linked List)
When storing an asset or commander record, tag it with the location
id and pull the display name/type via `graph_get_location_info`
rather than storing your own duplicate copy of the name — that way if
Member 1 renames or removes a node, your records don't go stale
silently (you can check `graph_node_exists` before displaying).

### Shreya (Member 4) — UI + Integration
Your `main()` owns the single `Graph *g` for the whole program,
created once at startup and passed into every module's entry point
(e.g. `logistics_init(g)`, `intel_init(g)` — coordinate exact
signatures with Members 2/3). Your menu simply dispatches to
`graph_bfs`, `graph_kruskal_mst`, `graph_fail_link`, etc. based on
user choice, then calls `graph_destroy(g)` on exit.

---

## 6. Build note

Only these `.c` files need to be compiled into the final executable
for the Graph module (never compile `main_test.c` into the real
program — it's Member 1's standalone test driver):

```
graph.c bfs.c dfs.c mst.c network_failure.c
```

Everyone links against `graph_api.h`; only these five `.c` files
`#include "graph_internal.h"`.
