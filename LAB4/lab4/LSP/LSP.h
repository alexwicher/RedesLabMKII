#include "../packet_m.h"
#include <tuple>
#include <vector>

#ifndef LSP_LSP_H_
#define LSP_LSP_H_



class LSP : public Packet {
private:
    int routerID;
    int ** neighCostList;
    int maxNeighs;
    int totalNodes;

public:
    LSP( int totalNodes,int maxNeighs);
    virtual ~LSP();
    int getRouterID();
    void setRouterID(int routerID);
    int ** getNeighCostList();
    void modNeighCostList(int i,int neighID,int cost);
    void setNeighCostList(int ** ncList);
    int getMaxNeighs();
    int getTotalNodes();
    void setMaxNeighs(int maxNei);
    void setTotalNodes(int maxNodes);
    LSP* dup();
//    int getFromVector(int i, int j);
//    bool isVecEmptyAt(int i);
};

#endif /* LSP_LSP_H_ */
