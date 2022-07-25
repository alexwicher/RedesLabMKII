#include "../Dijkstra/Dijkstra.h"

#include <cstdlib>
#include <iostream>
#include <limits.h>
#include <stdio.h>


// Based on https://www.geeksforgeeks.org/c-program-for-dijkstras-shortest-path-algorithm-greedy-algo-7/
// and
// and https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm


Dijkstra::Dijkstra(int totalNodes) {
    this->totalNodes = totalNodes;
    this->dist = (int*)malloc(totalNodes * sizeof(int));
    this->sptSet  = (bool*)malloc(totalNodes * sizeof(bool));
    this->paths =(int*)malloc(totalNodes * sizeof(int));
}

Dijkstra::~Dijkstra() {
    free(this->sptSet);
    free(this->dist);
    free(this->paths);
}

int Dijkstra::minDistance(int *dist, bool *sptSet) {
    int min = INT_MAX, min_index;
    for (int v = 0; v < totalNodes; v++){
        if (sptSet[v] == false && dist[v] <= min)
        min = dist[v], min_index = v;
    }
    return min_index;
}

void Dijkstra::dijkstra(int ** graph, int src) {
    for (int i = 0; i < totalNodes; i++)
        dist[i] = INT_MAX, sptSet[i] = false, paths[i] = -1;
    dist[src] = 0;
    for (int count = 0; count < totalNodes - 1; count++) {
        int u = minDistance(dist, sptSet);
        sptSet[u] = true;
        for (int v = 0; v < totalNodes; v++)
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
                paths[v] = u;
        }
    }
}


int Dijkstra::getTotalNodes(){
    return totalNodes;
}
void Dijkstra::setTotalNodes(int totalNodes){
    this->totalNodes=totalNodes;
}

int* Dijkstra::getPaths(){
    return paths;
}

int* Dijkstra::getDist(){
    return dist;
}





