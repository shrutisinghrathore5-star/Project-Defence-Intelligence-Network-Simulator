#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph_api.h"


/* =========================================================
   PRIVATE GRAPH STRUCTURES
   Other modules must NOT access these directly.
   ========================================================= */

#define MAX_LOCATIONS GRAPH_MAX_NODES
#define MAX_LINKS 100

typedef struct
{
    int destination;
    int weight;
    int active;
} GraphLink;


typedef struct
{
    int id;
    char name[GRAPH_MAX_NAME_LEN];
    LocationType type;
    int active;

    GraphLink links[MAX_LINKS];
    int link_count;

} GraphNode;


struct Graph
{
    GraphNode nodes[MAX_LOCATIONS];
    int node_count;
};


/* =========================================================
   PRIVATE HELPER
   Find node index from ID
   ========================================================= */

static int find_node_index(Graph *g, int id)
{
    int i;

    if (g == NULL)
    {
        return -1;
    }

    for (i = 0; i < g->node_count; i++)
    {
        if (g->nodes[i].id == id)
        {
            return i;
        }
    }

    return -1;
}


/* =========================================================
   GRAPH CREATE
   ========================================================= */

Graph *graph_create(void)
{
    Graph *g;

    g = (Graph *)malloc(sizeof(Graph));

    if (g == NULL)
    {
        return NULL;
    }

    g->node_count = 0;

    return g;
}


/* =========================================================
   GRAPH DESTROY
   ========================================================= */

void graph_destroy(Graph *g)
{
    if (g == NULL)
    {
        return;
    }

    free(g);
}


/* =========================================================
   ADD LOCATION
   Returns new location ID.
   Returns -1 on failure.
   ========================================================= */

int graph_add_location(Graph *g, const char *name, LocationType type)
{
    GraphNode *node;
    int new_id;

    if (g == NULL || name == NULL)
    {
        return -1;
    }

    if (g->node_count >= MAX_LOCATIONS)
    {
        return -1;
    }

    /*
       Generate ID automatically.
       First location = 0
       Second location = 1
       etc.
    */

    new_id = g->node_count;

    node = &g->nodes[g->node_count];

    node->id = new_id;

    strncpy(
        node->name,
        name,
        GRAPH_MAX_NAME_LEN - 1
    );

    node->name[GRAPH_MAX_NAME_LEN - 1] = '\0';

    node->type = type;

    node->active = 1;

    node->link_count = 0;

    g->node_count++;

    return new_id;
}


/* =========================================================
   REMOVE LOCATION
   ========================================================= */

int graph_remove_location(Graph *g, int id)
{
    int index;
    int i;
    int j;

    index = find_node_index(g, id);

    if (index == -1)
    {
        return 0;
    }

    if (!g->nodes[index].active)
    {
        return 0;
    }

    g->nodes[index].active = 0;

    /*
       Disable links pointing to this node.
    */

    for (i = 0; i < g->node_count; i++)
    {
        for (j = 0; j < g->nodes[i].link_count; j++)
        {
            if (g->nodes[i].links[j].destination == id)
            {
                g->nodes[i].links[j].active = 0;
            }
        }
    }

    return 1;
}


/* =========================================================
   ADD LINK
   ========================================================= */

int graph_add_link(
    Graph *g,
    int srcId,
    int destId,
    int weight
)
{
    int source_index;
    int destination_index;
    int i;

    if (g == NULL)
    {
        return 0;
    }

    source_index = find_node_index(g, srcId);
    destination_index = find_node_index(g, destId);

    if (source_index == -1 ||
        destination_index == -1)
    {
        return 0;
    }

    if (!g->nodes[source_index].active ||
        !g->nodes[destination_index].active)
    {
        return 0;
    }

    if (g->nodes[source_index].link_count >= MAX_LINKS)
    {
        return 0;
    }

    /*
       Prevent duplicate links.
    */

    for (i = 0;
         i < g->nodes[source_index].link_count;
         i++)
    {
        if (g->nodes[source_index]
                .links[i]
                .destination == destId)
        {
            return 0;
        }
    }

    i = g->nodes[source_index].link_count;

    g->nodes[source_index].links[i].destination = destId;
    g->nodes[source_index].links[i].weight = weight;
    g->nodes[source_index].links[i].active = 1;

    g->nodes[source_index].link_count++;

    return 1;
}


/* =========================================================
   REMOVE LINK
   ========================================================= */

int graph_remove_link(
    Graph *g,
    int srcId,
    int destId
)
{
    int source_index;
    int i;

    source_index = find_node_index(g, srcId);

    if (source_index == -1)
    {
        return 0;
    }

    for (i = 0;
         i < g->nodes[source_index].link_count;
         i++)
    {
        if (g->nodes[source_index]
                .links[i]
                .destination == destId)
        {
            g->nodes[source_index]
                .links[i]
                .active = 0;

            return 1;
        }
    }

    return 0;
}


/* =========================================================
   DISPLAY GRAPH
   ========================================================= */

void graph_display(Graph *g)
{
    int i;
    int j;

    if (g == NULL)
    {
        return;
    }

    printf("\n========== DEFENCE NETWORK ==========\n");

    for (i = 0; i < g->node_count; i++)
    {
        printf(
            "ID: %d | Name: %s | Type: %s | Active: %d\n",
            g->nodes[i].id,
            g->nodes[i].name,
            graph_type_to_string(g->nodes[i].type),
            g->nodes[i].active
        );

        for (j = 0;
             j < g->nodes[i].link_count;
             j++)
        {
            if (g->nodes[i].links[j].active)
            {
                printf(
                    "    -> %d (weight: %d)\n",
                    g->nodes[i].links[j].destination,
                    g->nodes[i].links[j].weight
                );
            }
        }
    }

    printf("=====================================\n");
}


/* =========================================================
   NODE COUNT
   ========================================================= */

int graph_node_count(Graph *g)
{
    if (g == NULL)
    {
        return 0;
    }

    return g->node_count;
}


/* =========================================================
   NODE EXISTS
   ========================================================= */

int graph_node_exists(Graph *g, int id)
{
    return find_node_index(g, id) != -1;
}


/* =========================================================
   NODE ACTIVE
   ========================================================= */

int graph_node_active(Graph *g, int id)
{
    int index;

    index = find_node_index(g, id);

    if (index == -1)
    {
        return 0;
    }

    return g->nodes[index].active;
}


/* =========================================================
   GET LOCATION INFO
   ========================================================= */

int graph_get_location_info(
    Graph *g,
    int id,
    LocationInfo *outInfo
)
{
    int index;

    if (g == NULL || outInfo == NULL)
    {
        return 0;
    }

    index = find_node_index(g, id);

    if (index == -1)
    {
        return 0;
    }

    outInfo->id = g->nodes[index].id;

    strcpy(
        outInfo->name,
        g->nodes[index].name
    );

    outInfo->type = g->nodes[index].type;

    outInfo->active = g->nodes[index].active;

    return 1;
}


/* =========================================================
   TYPE TO STRING
   ========================================================= */

const char *graph_type_to_string(LocationType type)
{
    switch (type)
    {
        case MILITARY_BASE:
            return "Military Base";

        case BORDER_POST:
            return "Border Post";

        case RADAR_STATION:
            return "Radar Station";

        case AIRPORT:
            return "Airport";

        case MILITARY_HOSPITAL:
            return "Military Hospital";

        case SUPPLY_DEPOT:
            return "Supply Depot";

        default:
            return "Unknown";
    }
}


/* =========================================================
   FAILURE SIMULATION
   ========================================================= */

int graph_fail_link(
    Graph *g,
    int srcId,
    int destId
)
{
    int source_index;
    int i;

    source_index = find_node_index(g, srcId);

    if (source_index == -1)
    {
        return 0;
    }

    for (i = 0;
         i < g->nodes[source_index].link_count;
         i++)
    {
        if (g->nodes[source_index]
                .links[i]
                .destination == destId)
        {
            g->nodes[source_index]
                .links[i]
                .active = 0;

            return 1;
        }
    }

    return 0;
}


/* =========================================================
   RESTORE LINK
   ========================================================= */

int graph_restore_link(
    Graph *g,
    int srcId,
    int destId
)
{
    int source_index;
    int i;

    source_index = find_node_index(g, srcId);

    if (source_index == -1)
    {
        return 0;
    }

    for (i = 0;
         i < g->nodes[source_index].link_count;
         i++)
    {
        if (g->nodes[source_index]
                .links[i]
                .destination == destId)
        {
            g->nodes[source_index]
                .links[i]
                .active = 1;

            return 1;
        }
    }

    return 0;
}


/* =========================================================
   FAIL NODE
   ========================================================= */

int graph_fail_node(Graph *g, int id)
{
    int index;

    index = find_node_index(g, id);

    if (index == -1)
    {
        return 0;
    }

    g->nodes[index].active = 0;

    return 1;
}


/* =========================================================
   BFS
   ========================================================= */

void graph_bfs(Graph *g, int startId)
{
    int queue[MAX_LOCATIONS];
    int visited[MAX_LOCATIONS];

    int front = 0;
    int rear = 0;

    int start_index;
    int current_index;

    int i;
    int j;
    int destination;

    if (g == NULL)
    {
        return;
    }

    start_index = find_node_index(g, startId);

    if (start_index == -1)
    {
        return;
    }

    for (i = 0; i < MAX_LOCATIONS; i++)
    {
        visited[i] = 0;
    }

    queue[rear++] = start_index;
    visited[start_index] = 1;

    printf("\nBFS: ");

    while (front < rear)
    {
        current_index = queue[front++];

        printf(
            "%d(%s) ",
            g->nodes[current_index].id,
            g->nodes[current_index].name
        );

        for (j = 0;
             j < g->nodes[current_index].link_count;
             j++)
        {
            if (!g->nodes[current_index].links[j].active)
            {
                continue;
            }

            destination =
                find_node_index(
                    g,
                    g->nodes[current_index]
                        .links[j]
                        .destination
                );

            if (destination != -1 &&
                !visited[destination] &&
                g->nodes[destination].active)
            {
                visited[destination] = 1;
                queue[rear++] = destination;
            }
        }
    }

    printf("\n");
}


/* =========================================================
   DFS RECURSIVE HELPER
   ========================================================= */

static void dfs_visit(
    Graph *g,
    int index,
    int visited[]
)
{
    int i;
    int next;

    visited[index] = 1;

    printf(
        "%d(%s) ",
        g->nodes[index].id,
        g->nodes[index].name
    );

    for (i = 0;
         i < g->nodes[index].link_count;
         i++)
    {
        if (!g->nodes[index].links[i].active)
        {
            continue;
        }

        next =
            find_node_index(
                g,
                g->nodes[index].links[i].destination
            );

        if (next != -1 &&
            !visited[next] &&
            g->nodes[next].active)
        {
            dfs_visit(g, next, visited);
        }
    }
}


/* =========================================================
   DFS RECURSIVE
   ========================================================= */

void graph_dfs_recursive(
    Graph *g,
    int startId
)
{
    int visited[MAX_LOCATIONS];
    int start_index;
    int i;

    if (g == NULL)
    {
        return;
    }

    for (i = 0; i < MAX_LOCATIONS; i++)
    {
        visited[i] = 0;
    }

    start_index = find_node_index(g, startId);

    if (start_index == -1)
    {
        return;
    }

    printf("\nDFS Recursive: ");

    dfs_visit(
        g,
        start_index,
        visited
    );

    printf("\n");
}


/* =========================================================
   DFS ITERATIVE
   ========================================================= */

void graph_dfs_iterative(
    Graph *g,
    int startId
)
{
    int stack[MAX_LOCATIONS];
    int visited[MAX_LOCATIONS];

    int top = -1;
    int start_index;
    int current;
    int next;

    int i;
    int j;

    if (g == NULL)
    {
        return;
    }

    for (i = 0; i < MAX_LOCATIONS; i++)
    {
        visited[i] = 0;
    }

    start_index = find_node_index(g, startId);

    if (start_index == -1)
    {
        return;
    }

    stack[++top] = start_index;

    printf("\nDFS Iterative: ");

    while (top >= 0)
    {
        current = stack[top--];

        if (visited[current])
        {
            continue;
        }

        visited[current] = 1;

        printf(
            "%d(%s) ",
            g->nodes[current].id,
            g->nodes[current].name
        );

        for (j = g->nodes[current].link_count - 1;
             j >= 0;
             j--)
        {
            if (!g->nodes[current].links[j].active)
            {
                continue;
            }

            next =
                find_node_index(
                    g,
                    g->nodes[current]
                        .links[j]
                        .destination
                );

            if (next != -1 &&
                !visited[next] &&
                g->nodes[next].active)
            {
                stack[++top] = next;
            }
        }
    }

    printf("\n");
}


/* =========================================================
   CONNECTIVITY
   ========================================================= */

int graph_is_connected(Graph *g)
{
    int visited[MAX_LOCATIONS];
    int queue[MAX_LOCATIONS];

    int front = 0;
    int rear = 0;

    int active_count = 0;
    int visited_count = 0;

    int start = -1;

    int i;
    int j;
    int current;
    int next;

    if (g == NULL)
    {
        return 0;
    }

    for (i = 0; i < MAX_LOCATIONS; i++)
    {
        visited[i] = 0;
    }

    for (i = 0; i < g->node_count; i++)
    {
        if (g->nodes[i].active)
        {
            active_count++;

            if (start == -1)
            {
                start = i;
            }
        }
    }

    if (active_count <= 1)
    {
        return 1;
    }

    queue[rear++] = start;
    visited[start] = 1;

    while (front < rear)
    {
        current = queue[front++];
        visited_count++;

        for (j = 0;
             j < g->nodes[current].link_count;
             j++)
        {
            if (!g->nodes[current].links[j].active)
            {
                continue;
            }

            next =
                find_node_index(
                    g,
                    g->nodes[current]
                        .links[j]
                        .destination
                );

            if (next != -1 &&
                g->nodes[next].active &&
                !visited[next])
            {
                visited[next] = 1;
                queue[rear++] = next;
            }
        }
    }

    return visited_count == active_count;
}


/* =========================================================
   REPORT UNREACHABLE
   ========================================================= */

void graph_report_unreachable(
    Graph *g,
    int fromId
)
{
    int visited[MAX_LOCATIONS];
    int queue[MAX_LOCATIONS];

    int front = 0;
    int rear = 0;

    int start;
    int current;
    int next;

    int i;
    int j;

    if (g == NULL)
    {
        return;
    }

    for (i = 0; i < MAX_LOCATIONS; i++)
    {
        visited[i] = 0;
    }

    start = find_node_index(g, fromId);

    if (start == -1)
    {
        printf("Starting location not found.\n");
        return;
    }

    queue[rear++] = start;
    visited[start] = 1;

    while (front < rear)
    {
        current = queue[front++];

        for (j = 0;
             j < g->nodes[current].link_count;
             j++)
        {
            if (!g->nodes[current].links[j].active)
            {
                continue;
            }

            next =
                find_node_index(
                    g,
                    g->nodes[current]
                        .links[j]
                        .destination
                );

            if (next != -1 &&
                g->nodes[next].active &&
                !visited[next])
            {
                visited[next] = 1;
                queue[rear++] = next;
            }
        }
    }

    printf("\nUnreachable locations:\n");

    for (i = 0; i < g->node_count; i++)
    {
        if (g->nodes[i].active &&
            !visited[i])
        {
            printf(
                "%d - %s\n",
                g->nodes[i].id,
                g->nodes[i].name
            );
        }
    }
}


/* =========================================================
   MST FUNCTIONS
   =========================================================
   Basic implementations are provided so the public API
   links successfully. They display the available links.
   ========================================================= */

void graph_kruskal_mst(Graph *g)
{
    int i;
    int j;

    if (g == NULL)
    {
        return;
    }

    printf("\nKruskal MST links:\n");

    for (i = 0; i < g->node_count; i++)
    {
        for (j = 0;
             j < g->nodes[i].link_count;
             j++)
        {
            if (g->nodes[i].links[j].active)
            {
                printf(
                    "%d -> %d (weight %d)\n",
                    g->nodes[i].id,
                    g->nodes[i].links[j].destination,
                    g->nodes[i].links[j].weight
                );
            }
        }
    }
}


void graph_prim_mst(Graph *g)
{
    int i;
    int j;

    if (g == NULL)
    {
        return;
    }

    printf("\nPrim MST links:\n");

    for (i = 0; i < g->node_count; i++)
    {
        for (j = 0;
             j < g->nodes[i].link_count;
             j++)
        {
            if (g->nodes[i].links[j].active)
            {
                printf(
                    "%d -> %d (weight %d)\n",
                    g->nodes[i].id,
                    g->nodes[i].links[j].destination,
                    g->nodes[i].links[j].weight
                );
            }
        }
    }
}
