#include <stdio.h>
#include "graph_internal.h"

/* ------------------------------------------------------------
   BFS uses a QUEUE (FIFO) - syllabus topic "Queues".
   We implement a small circular queue of ints right here so the
   traversal doesn't depend on Member 2's Queue module (keeps
   Graph module self-contained/compilable on its own), but the
   *interface* (enqueue/dequeue/isEmpty) is identical to what a
   generic Queue ADT would expose.
   ------------------------------------------------------------ */
typedef struct {
    int items[MAX_NODES];
    int front, rear, size;
} SimpleQueue;

static void qInit(SimpleQueue *q)   { q->front = 0; q->rear = -1; q->size = 0; }
static int  qEmpty(SimpleQueue *q)  { return q->size == 0; }
static void qEnqueue(SimpleQueue *q, int val) {
    q->rear = (q->rear + 1) % MAX_NODES;
    q->items[q->rear] = val;
    q->size++;
}
static int qDequeue(SimpleQueue *q) {
    int val = q->items[q->front];
    q->front = (q->front + 1) % MAX_NODES;
    q->size--;
    return val;
}

/* Time complexity: O(V + E)  (each active node enqueued once, each
   active edge inspected once)
   Space complexity: O(V) for visited[] + queue */
void graph_bfs(Graph *g, int startId) {
    if (!g->nodeExists[startId] || !g->nodes[startId].active) {
        printf("BFS: start node invalid or offline.\n");
        return;
    }

    int visited[MAX_NODES] = {0};
    SimpleQueue q;
    qInit(&q);

    visited[startId] = 1;
    qEnqueue(&q, startId);

    printf("\nBFS from '%s': ", g->nodes[startId].name);
    while (!qEmpty(&q)) {
        int curr = qDequeue(&q);
        printf("%s ", g->nodes[curr].name);

        AdjNode *edge = g->adjList[curr];
        while (edge) {
            int nb = edge->destId;
            if (edge->linkActive && g->nodes[nb].active && !visited[nb]) {
                visited[nb] = 1;
                qEnqueue(&q, nb);
            }
            edge = edge->next;
        }
    }
    printf("\n(Time: O(V+E), Space: O(V))\n");
}
