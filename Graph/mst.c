#include <stdio.h>
#include <stdlib.h>
#include "graph_internal.h"

/* ============================================================
   KRUSKAL'S MST
   Combines TWO syllabus topics in one algorithm:
     - Sorting        : edges sorted by weight (qsort)
     - Disjoint Set    : Union-Find (with path compression +
                          union by rank) to detect cycles in O(a(V))
   Overall time: O(E log E) for the sort + O(E a(V)) for union-find
                 ~ O(E log E) dominant term
   Space: O(V) for the union-find arrays + O(E) for the edge list
   ============================================================ */

/* ---- Disjoint Set (Union-Find) ---- */
typedef struct {
    int parent[MAX_NODES];
    int rank[MAX_NODES];
} DisjointSet;

static void dsInit(DisjointSet *ds, int n) {
    for (int i = 0; i < n; i++) { ds->parent[i] = i; ds->rank[i] = 0; }
}
static int dsFind(DisjointSet *ds, int x) {
    if (ds->parent[x] != x)
        ds->parent[x] = dsFind(ds, ds->parent[x]); /* path compression */
    return ds->parent[x];
}
static void dsUnion(DisjointSet *ds, int a, int b) {
    int ra = dsFind(ds, a), rb = dsFind(ds, b);
    if (ra == rb) return;
    if (ds->rank[ra] < ds->rank[rb]) ds->parent[ra] = rb;
    else if (ds->rank[ra] > ds->rank[rb]) ds->parent[rb] = ra;
    else { ds->parent[rb] = ra; ds->rank[ra]++; }
}

static int compareEdges(const void *a, const void *b) {
    return ((Edge *)a)->weight - ((Edge *)b)->weight;
}

void graph_kruskal_mst(Graph *g) {
    Edge edges[MAX_NODES * MAX_NODES];
    int edgeCount = getEdgeList(g, edges, MAX_NODES * MAX_NODES);
    qsort(edges, edgeCount, sizeof(Edge), compareEdges); /* Sorting topic */

    DisjointSet ds;
    dsInit(&ds, g->nextId);

    int totalCost = 0, edgesUsed = 0;
    printf("\n===== MINIMUM SPANNING TREE (Kruskal's) =====\n");
    for (int i = 0; i < edgeCount && edgesUsed < g->numNodes - 1; i++) {
        int u = edges[i].src, v = edges[i].dest;
        if (!g->nodes[u].active || !g->nodes[v].active) continue;
        if (dsFind(&ds, u) != dsFind(&ds, v)) {
            dsUnion(&ds, u, v);
            printf("  %s -- %s  (cost %d)\n",
                   g->nodes[u].name, g->nodes[v].name, edges[i].weight);
            totalCost += edges[i].weight;
            edgesUsed++;
        }
    }
    if (edgesUsed < g->numNodes - 1)
        printf("  WARNING: network is disconnected, MST covers only part of it.\n");
    printf("Total minimum secure-link cost: %d\n", totalCost);
    printf("(Time: O(E log E), Space: O(V+E))\n");
}

/* ============================================================
   PRIM'S MST
   Classic array-based version (no heap needed): maintain a
   minKey[] / inMST[] array and linearly scan for the cheapest
   frontier edge each round. O(V^2) time - perfectly fine for a
   campus-scale network (a few hundred nodes) and keeps the
   implementation dependency-free.
   ============================================================ */
void graph_prim_mst(Graph *g) {
    int n = g->nextId;
    int inMST[MAX_NODES] = {0};
    int minKey[MAX_NODES];
    int parent[MAX_NODES];

    int start = -1;
    for (int i = 0; i < n; i++) {
        minKey[i] = INF; parent[i] = -1;
        if (g->nodeExists[i] && g->nodes[i].active && start == -1) start = i;
    }
    if (start == -1) { printf("Prim's: no active nodes.\n"); return; }
    minKey[start] = 0;

    printf("\n===== MINIMUM SPANNING TREE (Prim's) =====\n");
    int totalCost = 0;

    for (int count = 0; count < g->numNodes; count++) {
        /* pick the un-included active node with smallest key */
        int u = -1, best = INF;
        for (int v = 0; v < n; v++) {
            if (g->nodeExists[v] && g->nodes[v].active && !inMST[v] && minKey[v] < best) {
                best = minKey[v]; u = v;
            }
        }
        if (u == -1) break; /* remaining nodes unreachable */
        inMST[u] = 1;
        if (parent[u] != -1) {
            printf("  %s -- %s  (cost %d)\n",
                   g->nodes[parent[u]].name, g->nodes[u].name, minKey[u]);
            totalCost += minKey[u];
        }

        AdjNode *edge = g->adjList[u];
        while (edge) {
            int v = edge->destId;
            if (edge->linkActive && g->nodes[v].active && !inMST[v] && edge->weight < minKey[v]) {
                minKey[v] = edge->weight;
                parent[v] = u;
            }
            edge = edge->next;
        }
    }
    printf("Total minimum secure-link cost: %d\n", totalCost);
    printf("(Time: O(V^2), Space: O(V))\n");
}
