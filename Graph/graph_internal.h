#ifndef GRAPH_INTERNAL_H
#define GRAPH_INTERNAL_H

#include "graph_api.h"

/* ============================================================
   PRIVATE to the Graph module. graph.c, bfs.c, dfs.c, mst.c and
   network_failure.c all include this to see the real fields.
   Nothing outside this folder should ever include this file.
   ============================================================ */

#define MAX_NODES    GRAPH_MAX_NODES
#define MAX_NAME_LEN GRAPH_MAX_NAME_LEN
#define INF          999999

typedef struct LocationInternal {
    int id;
    char name[MAX_NAME_LEN];
    LocationType type;
    int active;
} LocationInternal;

typedef struct AdjNode {
    int destId;
    int weight;
    int linkActive;
    struct AdjNode *next;
} AdjNode;

/* This is the REAL definition matching the opaque `struct Graph`
   declared in graph_api.h. */
struct Graph {
    LocationInternal nodes[MAX_NODES];
    AdjNode  *adjList[MAX_NODES];
    int      nodeExists[MAX_NODES];
    int      numNodes;
    int      nextId;
};

typedef struct Edge {
    int src;
    int dest;
    int weight;
} Edge;

/* internal-only helper, used by mst.c */
int getEdgeList(Graph *g, Edge edges[], int maxEdges);

#endif
