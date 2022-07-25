#include "LSPRouter.h"
#include <cstdlib>



LSPRouter::LSPRouter(int totalNodes,int maxNeighs) {
    this->LSPCount = 0;
    this->totalNodes = totalNodes;
    this->maxNeighs = maxNeighs;

    adjMatrix = (int**)calloc(totalNodes ,sizeof(int*));
    lspDB = (LSP**)malloc(totalNodes * sizeof(LSP*));
    pathStore = (int**)malloc(totalNodes * sizeof(int*));

    for (int i = 0; i < totalNodes; ++i){
        adjMatrix[i] = (int*)calloc (totalNodes , sizeof(int));
        lspDB[i] = (LSP*)malloc (totalNodes * sizeof(LSP));
        lspDB[i] =  NULL;

        pathStore[i] = (int*)malloc (totalNodes * sizeof(int));
        pathStore[i]= NULL;
    }

    neighList = (int*)malloc (maxNeighs *sizeof(int));
    for (int i = 0; i < maxNeighs; ++i)
        neighList[i] = -1;
}

LSPRouter::~LSPRouter() {
    for (int i = 0; i < totalNodes; ++i){
        free(this->adjMatrix[i]);
        free(this->lspDB[i]);
        free(this->pathStore[i]);
    }
    free(this->adjMatrix);
    free(this->lspDB);
    free(this->neighList);
    free(this->pathStore);
}

void LSPRouter::setRouterID(int id){
    routerID = id;
}
int LSPRouter::getRouterID(){
    return routerID;
}
void LSPRouter::modAdjMatrix(int i, int j, int weight){
    adjMatrix[i][j] = weight;
}
void LSPRouter::modNeighList(int i,int neighID){
    neighList[i] = neighID ;
}

int LSPRouter::getMaxNeighs(){
    return maxNeighs;
}

int LSPRouter::getTotalNodes(){
    return totalNodes;
}

int* LSPRouter::getNeighList(){
    return neighList;
}

int** LSPRouter::getAdjMatrix(){
    return adjMatrix;
}

LSP** LSPRouter::getLspDB(){
    return lspDB;
}
void LSPRouter::setLspDB(LSP** lspDb){
    lspDB = lspDb;
}

void LSPRouter::addToLSPDB(LSP* lsp){
    if(LSPCount < totalNodes){
        lspDB[lsp->getRouterID()] = lsp;
        LSPCount++;
    }
}

int LSPRouter::getLSPCounter(){
    return LSPCount;
}

bool LSPRouter::isLSPGoodToAdd(LSP* lsp){
    if(lspDB[lsp->getRouterID()] == NULL)
        return true;
//    else if(lspDB[lsp->getRouterID()]->getCreationTime() < lsp->getCreationTime()){
//        return true;
//    }
    return false;
}


void LSPRouter::printAdjMatrix() {
  for (int i = 0; i < totalNodes; i++) {
    std::cout << i << " : ";
    for (int j = 0; j < totalNodes; j++)
      std::cout << adjMatrix[i][j] << " ";
    std::cout << "\n";
  }
}

int ** LSPRouter::getPathStore(){
    return this->pathStore;
}

void LSPRouter::setPathStore(int** pathStore){
    this->pathStore = pathStore;
}
void  LSPRouter::modPathStore(int i , int* path){
    this->pathStore[i] = path;
}
