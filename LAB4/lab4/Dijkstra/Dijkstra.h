#ifndef DIJKSTRA_DIJKSTRA_H_
#define DIJKSTRA_DIJKSTRA_H_

#include <queue>

class Dijkstra {
private:
    int totalNodes;
    int minDistance(int * dist, bool * sptSet);
    void printSolution(int * dist);
    int* dist;
    int* paths;
    bool* sptSet;

    std::queue<int> priority_Q;
public:
    Dijkstra(int totalNodes);
    virtual ~Dijkstra();
    void dijkstra(int ** graph, int src);
    int getTotalNodes();
    void setTotalNodes(int totalNodes);
    int* getPaths();
    int* getDist();
};

#endif /* DIJKSTRA_DIJKSTRA_H_ */
