#include <stdio.h>
#include "graph_internal.h"

/* ------------------------------------------------------------
   Simulates real-world attrition: a comms link gets jammed/cut,
   or a base gets knocked out. We DON'T delete the underlying
   edge/node data (so it can be "restored" later) - we just flip
   an active/linkActive flag. Connectivity is then re-verified
   with a BFS scan, reusing the same O(V+E) traversal already
   built for the Graph module - a good example of algorithm reuse.
   ------------------------------------------------------------ */

int graph_fail_link(Graph *g, int srcId, int destId) {
    int found = 0;
    AdjNode *curr = g->adjList[srcId];
    while (curr) {
        if (curr->destId == destId) { curr->linkActive = 0; found = 1; break; }
        curr = curr->next;
    }
    curr = g->adjList[destId];
    while (curr) {
        if (curr->destId == srcId) { curr->linkActive = 0; break; }
        curr = curr->next;
    }
    if (found) printf("ALERT: link %s <-> %s is DOWN.\n",
                       g->nodes[srcId].name, g->nodes[destId].name);
    return found;
}

int graph_restore_link(Graph *g, int srcId, int destId) {
    int found = 0;
    AdjNode *curr = g->adjList[srcId];
    while (curr) {
        if (curr->destId == destId) { curr->linkActive = 1; found = 1; break; }
        curr = curr->next;
    }
    curr = g->adjList[destId];
    while (curr) {
        if (curr->destId == srcId) { curr->linkActive = 1; break; }
        curr = curr->next;
    }
    return found;
}

int graph_fail_node(Graph *g, int id) {
    if (!g->nodeExists[id]) return 0;
    g->nodes[id].active = 0;
    printf("ALERT: %s is OFFLINE (destroyed/captured).\n", g->nodes[id].name);
    return 1;
}

/* Runs a BFS from the first active node and compares the count of
   nodes reached against the total number of currently active nodes.
   Returns 1 if fully connected, 0 otherwise.
   Time: O(V+E)  Space: O(V) */
int graph_is_connected(Graph *g) {
    int start = -1;
    int totalActive = 0;
    for (int i = 0; i < g->nextId; i++) {
        if (g->nodeExists[i] && g->nodes[i].active) {
            totalActive++;
            if (start == -1) start = i;
        }
    }
    if (start == -1) return 1; /* no active nodes -> trivially "connected" */

    int visited[MAX_NODES] = {0};
    int stack[MAX_NODES], top = -1;
    stack[++top] = start;
    visited[start] = 1;
    int reached = 1;

    while (top >= 0) {
        int curr = stack[top--];
        AdjNode *edge = g->adjList[curr];
        while (edge) {
            int nb = edge->destId;
            if (edge->linkActive && g->nodes[nb].active && !visited[nb]) {
                visited[nb] = 1;
                reached++;
                stack[++top] = nb;
            }
            edge = edge->next;
        }
    }

    return reached == totalActive;
}

/* Lists every active node NOT reachable from fromId - useful after
   a failure to know which bases just got cut off from HQ. */
void graph_report_unreachable(Graph *g, int fromId) {
    if (!g->nodeExists[fromId] || !g->nodes[fromId].active) {
        printf("Reference node invalid/offline.\n");
        return;
    }
    int visited[MAX_NODES] = {0};
    int stack[MAX_NODES], top = -1;
    stack[++top] = fromId;
    visited[fromId] = 1;

    while (top >= 0) {
        int curr = stack[top--];
        AdjNode *edge = g->adjList[curr];
        while (edge) {
            int nb = edge->destId;
            if (edge->linkActive && g->nodes[nb].active && !visited[nb]) {
                visited[nb] = 1;
                stack[++top] = nb;
            }
            edge = edge->next;
        }
    }

    printf("\nNodes UNREACHABLE from %s:\n", g->nodes[fromId].name);
    int any = 0;
    for (int i = 0; i < g->nextId; i++) {
        if (g->nodeExists[i] && g->nodes[i].active && !visited[i]) {
            printf("  - %s\n", g->nodes[i].name);
            any = 1;
        }
    }
    if (!any) printf("  (none - network still fully connected)\n");
}
