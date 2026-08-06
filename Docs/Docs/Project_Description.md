# Project Sentinel: Defence Intelligence Network Simulator

> A pure-C simulation of a military communication and logistics network — built as a semester-long Data Structures PBL project. Not a CRUD app: every feature exists to demonstrate a specific data structure or algorithm in action.

## Overview

Project Sentinel models a network of military locations (bases, border posts, radar stations, airports, hospitals, supply depots) connected by roads and communication links. Users can build the network, simulate link/node failures, run graph algorithms to find optimal routes and minimum-cost connectivity, manage a mission queue, and search a searchable intelligence database — all through a menu-driven console application written in **pure ANSI C**, no C++, no STL, no OOP.

## Why this project

Most student DSA projects are CRUD wrappers around a database. Sentinel is algorithm-centric: each module exists *because* a specific data structure solves a real problem in the simulation, not the other way around.

| Data Structure / Algorithm | Where it's used | Why |
|---|---|---|
| Adjacency List (Arrays + Linked Lists) | Network representation | Sparse graph — O(V+E) space |
| Queue (circular array) | BFS traversal | Level-order network scan |
| Stack (explicit + recursive) | DFS traversal | Deep-route exploration, two implementations for comparison |
| Recursion | DFS, BST operations | Natural traversal expression |
| Disjoint Set (Union-Find) | Kruskal's MST | Cycle detection in O(α(V)) |
| Sorting (qsort) | Kruskal's MST | Edge ordering by weight |
| Priority Queue / Heap | Emergency mission scheduling | O(log n) highest-priority dispatch |
| Stack | Mission history | LIFO undo/audit trail |
| Hashing | Intel DB asset/commander lookup | O(1) average search |
| Binary Search Tree | Sorted intel records | O(log n) ordered search/insert |
| Graphs — BFS/DFS | Network traversal, connectivity checks | Route discovery, failure impact analysis |
| MST — Prim's & Kruskal's | Minimum-cost secure network design | Cheapest way to keep every base connected |

## Features

**Graph Engine**
- Create/remove military locations and communication links
- Display live network topology
- BFS and DFS traversal (recursive *and* explicit-stack versions)
- Minimum Spanning Tree via Kruskal's (sort + union-find) and Prim's
- Link failure / node failure simulation with automatic connectivity re-check

**Logistics Module**
- Mission queue and convoy scheduling
- Emergency mission prioritization via priority queue
- Mission history via stack

**Intelligence Database**
- Military asset and commander records
- O(1) average lookup via hash table
- Sorted record maintenance via BST

**Console UI**
- Single menu-driven executable integrating all modules

## Tech Stack

- **Language:** C (C11), compiled with GCC
- **Build:** GNU Make
- **No external libraries** beyond the C standard library

## Repository Structure

```
ProjectSentinel/
├── Graph/                  # Member 1 — Graph Engine
│   ├── graph_api.h         # public interface (opaque Graph type)
│   ├── graph_internal.h    # private struct layout
│   ├── graph.c             # core CRUD
│   ├── bfs.c
│   ├── dfs.c
│   ├── mst.c                # Kruskal's + Prim's
│   ├── network_failure.c
│   └── main_test.c         # standalone module test driver
├── Logistics/               # Member 2 — Queue, Priority Queue, Stack
├── IntelDB/                 # Member 3 — Hashing, BST, Linked List
├── UI/                      # Member 4 — menu-driven integration
├── docs/
│   └── INTEGRATION_CONTRACT.md
├── Makefile
└── README.md
```

## Build & Run

```bash
git clone https://github.com/<your-username>/ProjectSentinel.git
cd ProjectSentinel
make
./sentinel
```

To build and test the Graph module in isolation:

```bash
cd Graph
gcc graph.c bfs.c dfs.c mst.c network_failure.c main_test.c -o graph_test
./graph_test
```

## Team

| Member | Name | Responsibility |
|---|---|---|
| Member 1 (Team Lead) | Shruti Singh | Graph Engine — representation, BFS/DFS, MST, failure simulation |
| Member 2 | Shubhi Saxena | Logistics — Queue, Priority Queue, Stack, mission scheduling |
| Member 3 | Shiwani | Intel DB — Hashing, BST, Linked List, search/sort |
| Member 4 | Shreya | UI, file handling, integration, testing, documentation |

## License

Academic project — Data Structures using C, B.Tech AI & ML, Semester Project (PBL).
