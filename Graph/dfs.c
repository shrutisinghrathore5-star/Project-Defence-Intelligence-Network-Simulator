#include <stdio.h>
#include "graph_internal.h"

/* ------------------------------------------------------------
   Two versions on purpose:
     1) graph_dfs_recursive     -> demonstrates "Recursion" topic
        (relies on the implicit function call stack)
     2) graph_dfs_iterative -> demonstrates "Stacks and their
        applications" topic with an EXPLICIT stack ADT
   Both are O(V+E) time, O(V) space.
   ------------------------------------------------------------ */

static void dfsHelper(Graph *g, int curr, int visited[]) {
    visited[curr] = 1;
    printf("%s ", g->nodes[curr].name);
    AdjNode *edge = g->adjList[curr];
    while (edge) {
        int nb = edge->destId;
        if (edge->linkActive && g->nodes[nb].active && !visited[nb]) {
            dfsHelper(g, nb, visited);
        }
        edge = edge->next;
    }
}

void graph_dfs_recursive(Graph *g, int startId) {
    if (!g->nodeExists[startId] || !g->nodes[startId].active) {
        printf("DFS: start node invalid or offline.\n");
        return;
    }
    int visited[MAX_NODES] = {0};
    printf("\nDFS (recursive) from '%s': ", g->nodes[startId].name);
    dfsHelper(g, startId, visited);
    printf("\n(Time: O(V+E), Space: O(V) call stack)\n");
}

/* --- explicit stack ADT --- */
typedef struct {
    int items[MAX_NODES];
    int top; /* -1 = empty */
} SimpleStack;

static void sInit(SimpleStack *s)      { s->top = -1; }
static int  sEmpty(SimpleStack *s)     { return s->top == -1; }
static void sPush(SimpleStack *s, int v) { s->items[++(s->top)] = v; }
static int  sPop(SimpleStack *s)       { return s->items[(s->top)--]; }

void graph_dfs_iterative(Graph *g, int startId) {
    if (!g->nodeExists[startId] || !g->nodes[startId].active) {
        printf("DFS: start node invalid or offline.\n");
        return;
    }
    int visited[MAX_NODES] = {0};
    SimpleStack st;
    sInit(&st);
    sPush(&st, startId);

    printf("\nDFS (iterative, explicit Stack) from '%s': ", g->nodes[startId].name);
    while (!sEmpty(&st)) {
        int curr = sPop(&st);
        if (visited[curr]) continue;
        visited[curr] = 1;
        printf("%s ", g->nodes[curr].name);

        AdjNode *edge = g->adjList[curr];
        while (edge) {
            int nb = edge->destId;
            if (edge->linkActive && g->nodes[nb].active && !visited[nb]) {
                sPush(&st, nb);
            }
            edge = edge->next;
        }
    }
    printf("\n(Time: O(V+E), Space: O(V) explicit stack)\n");
}
