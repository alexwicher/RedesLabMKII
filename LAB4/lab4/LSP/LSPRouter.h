#ifndef LSP_LSPROUTER_H_
#define LSP_LSPROUTER_H_

#include <vector>
#include "LSP.h"

class LSPRouter {
private:
    int routerID;
    int** adjMatrix;
    int* neighList;
    int maxNeighs;
    int neighCount;
    int totalNodes;
    LSP** lspDB;
    int LSPCount;

public:
    LSPRouter(int totalNodes,int maxNeighs);
    virtual ~LSPRouter();
    void printAdjMatrix();
    int getRouterID();
    void setRouterID(int id);
    void modAdjMatrix(int i, int j, int weight);
    void modNeighList(int i,int neighID);
    int* getNeighList();
    int** getAdjMatrix();
    int getMaxNeighs();
    int getTotalNodes();
    LSP** getLspDB();
    void setLspDB(LSP** lspDb);
    void addToLSPDB(LSP* lsp);
    int getLSPCounter();
    bool isLSPGoodToAdd(LSP* lsp);
    int getNeighCount();
    void addNeighCount();
};

#endif /* LSP_LSPROUTER_H_ */
