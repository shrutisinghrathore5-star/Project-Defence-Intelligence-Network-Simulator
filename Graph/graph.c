#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph_internal.h"

/* ---------------- lifecycle ---------------- */

Graph *graph_create(void) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (!g) return NULL;
    g->numNodes = 0;
    g->nextId = 0;
    for (int i = 0; i < MAX_NODES; i++) {
        g->adjList[i] = NULL;
        g->nodeExists[i] = 0;
    }
    return g;
}

static void freeAdjList(AdjNode *head) {
    while (head) {
        AdjNode *tmp = head;
        head = head->next;
        free(tmp);
    }
}

void graph_destroy(Graph *g) {
    if (!g) return;
    for (int i = 0; i < g->nextId; i++) freeAdjList(g->adjList[i]);
    free(g);
}

const char *graph_type_to_string(LocationType t) {
    switch (t) {
        case MILITARY_BASE:      return "Military Base";
        case BORDER_POST:        return "Border Post";
        case RADAR_STATION:      return "Radar Station";
        case AIRPORT:            return "Airport";
        case MILITARY_HOSPITAL:  return "Military Hospital";
        case SUPPLY_DEPOT:       return "Supply Depot";
        default:                 return "Unknown";
    }
}

/* ---------------- core CRUD ---------------- */

int graph_add_location(Graph *g, const char *name, LocationType type) {
    if (g->nextId >= MAX_NODES) {
        printf("Error: node capacity reached.\n");
        return -1;
    }
    int id = g->nextId++;
    g->nodes[id].id = id;
    strncpy(g->nodes[id].name, name, MAX_NAME_LEN - 1);
    g->nodes[id].name[MAX_NAME_LEN - 1] = '\0';
    g->nodes[id].type = type;
    g->nodes[id].active = 1;
    g->nodeExists[id] = 1;
    g->adjList[id] = NULL;
    g->numNodes++;
    return id;
}

int graph_remove_location(Graph *g, int id) {
    if (id < 0 || id >= MAX_NODES || !g->nodeExists[id]) return 0;

    freeAdjList(g->adjList[id]);
    g->adjList[id] = NULL;

    for (int i = 0; i < g->nextId; i++) {
        if (!g->nodeExists[i] || i == id) continue;
        AdjNode *curr = g->adjList[i];
        AdjNode *prev = NULL;
        while (curr) {
            if (curr->destId == id) {
                AdjNode *toDelete = curr;
                if (prev) prev->next = curr->next;
                else      g->adjList[i] = curr->next;
                curr = curr->next;
                free(toDelete);
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }

    g->nodeExists[id] = 0;
    g->numNodes--;
    return 1;
}

int graph_add_link(Graph *g, int srcId, int destId, int weight) {
    if (srcId < 0 || destId < 0 || srcId >= MAX_NODES || destId >= MAX_NODES) return 0;
    if (!g->nodeExists[srcId] || !g->nodeExists[destId]) return 0;

    AdjNode *a = (AdjNode *)malloc(sizeof(AdjNode));
    a->destId = destId; a->weight = weight; a->linkActive = 1;
    a->next = g->adjList[srcId];
    g->adjList[srcId] = a;

    AdjNode *b = (AdjNode *)malloc(sizeof(AdjNode));
    b->destId = srcId; b->weight = weight; b->linkActive = 1;
    b->next = g->adjList[destId];
    g->adjList[destId] = b;

    return 1;
}

int graph_remove_link(Graph *g, int srcId, int destId) {
    if (!g->nodeExists[srcId] || !g->nodeExists[destId]) return 0;
    int removed = 0;

    AdjNode *curr = g->adjList[srcId], *prev = NULL;
    while (curr) {
        if (curr->destId == destId) {
            if (prev) prev->next = curr->next; else g->adjList[srcId] = curr->next;
            free(curr);
            removed = 1;
            break;
        }
        prev = curr; curr = curr->next;
    }

    curr = g->adjList[destId]; prev = NULL;
    while (curr) {
        if (curr->destId == srcId) {
            if (prev) prev->next = curr->next; else g->adjList[destId] = curr->next;
            free(curr);
            break;
        }
        prev = curr; curr = curr->next;
    }
    return removed;
}

void graph_display(Graph *g) {
    printf("\n===== DEFENCE NETWORK TOPOLOGY =====\n");
    for (int i = 0; i < g->nextId; i++) {
        if (!g->nodeExists[i]) continue;
        printf("[%d] %-20s (%-17s) %s\n", i, g->nodes[i].name,
               graph_type_to_string(g->nodes[i].type),
               g->nodes[i].active ? "ACTIVE" : "OFFLINE");
        AdjNode *curr = g->adjList[i];
        if (!curr) { printf("      -> (no links)\n"); continue; }
        while (curr) {
            printf("      -> %-20s [w=%d]%s\n",
                   g->nodes[curr->destId].name, curr->weight,
                   curr->linkActive ? "" : " (LINK DOWN)");
            curr = curr->next;
        }
    }
    printf("=====================================\n");
}

int getEdgeList(Graph *g, Edge edges[], int maxEdges) {
    int count = 0;
    for (int i = 0; i < g->nextId; i++) {
        if (!g->nodeExists[i]) continue;
        AdjNode *curr = g->adjList[i];
        while (curr) {
            if (i < curr->destId && curr->linkActive && count < maxEdges) {
                edges[count].src = i;
                edges[count].dest = curr->destId;
                edges[count].weight = curr->weight;
                count++;
            }
            curr = curr->next;
        }
    }
    return count;
}

/* ---------------- read-only queries for other modules ---------------- */

int graph_node_count(Graph *g) {
    return g->numNodes;
}

int graph_node_exists(Graph *g, int id) {
    if (id < 0 || id >= MAX_NODES) return 0;
    return g->nodeExists[id];
}

int graph_node_active(Graph *g, int id) {
    if (!graph_node_exists(g, id)) return 0;
    return g->nodes[id].active;
}

int graph_get_location_info(Graph *g, int id, LocationInfo *outInfo) {
    if (!graph_node_exists(g, id) || !outInfo) return 0;
    outInfo->id = g->nodes[id].id;
    strncpy(outInfo->name, g->nodes[id].name, GRAPH_MAX_NAME_LEN - 1);
    outInfo->name[GRAPH_MAX_NAME_LEN - 1] = '\0';
    outInfo->type = g->nodes[id].type;
    outInfo->active = g->nodes[id].active;
    return 1;
}
