#include "LSP.h"
#include <cstdlib>

LSP::LSP(int totalNodes,int maxNeighs) {
    neighCostList = (int**)calloc(maxNeighs ,sizeof(int*));
    for(int i = 0;i<maxNeighs;i++)
        neighCostList[i] = (int*)calloc(2 ,sizeof(int));

    this->setTotalNodes(totalNodes);
    this->setMaxNeighs(maxNeighs);
    this->setKind(6);
    this->setName("LSP");
}

LSP::~LSP() {
    for(int i = 0;i<maxNeighs;i++)
        free(neighCostList[i]);
    free(neighCostList);
}

int LSP::getRouterID(){
 return routerID;
}

void LSP::setRouterID(int routerID){
    this->routerID = routerID;
}

int ** LSP::getNeighCostList(){
    return neighCostList;
}

void LSP::modNeighCostList(int i,int neighID,int cost){
    neighCostList[i][0] = neighID;
    neighCostList[i][1] = cost;
}

void LSP::setNeighCostList(int ** ncList){
    this->neighCostList = ncList;
}

int LSP::getMaxNeighs(){
    return maxNeighs;
}

int LSP::getTotalNodes(){
    return totalNodes;
}

void LSP::setMaxNeighs(int maxNei){
    this->maxNeighs = maxNei;
}

void LSP::setTotalNodes(int maxNodes){
    this->totalNodes = maxNodes;
}

LSP* LSP::dup(){
    LSP* res = new LSP(totalNodes,maxNeighs);
    res->setRouterID(routerID);
    res->setHopCount(this->getHopCount());
    for(int i = 0; i < maxNeighs ; i++){
        res->modNeighCostList(i, neighCostList[i][0], neighCostList[i][1]);
    }
    return res;
}

