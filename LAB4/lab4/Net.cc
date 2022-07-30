#ifndef NET
#define NET

#define HELLO_BYTE_SIZE 8
#define HELLO_ACK_BYTE_SIZE 8
#define ECHO_BYTE_SIZE 8


#define ECHO_TIMER 15

#define ECHO_WEIGHT_FACTOR 4

#define LSP_BYTE_SIZE 16 + 16*MAX_INTERFACES


#define TOTAL_NODES 8
#define INTERFACES 2


#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>
#include "LSP/LSPRouter.h"
#include "LSP/LSP.h"
#include "Dijkstra/Dijkstra.h"

#include<unordered_map>

using namespace omnetpp;

class Net: public cSimpleModule {
private:
    int nodeID;
    bool netGrapthUp = false;
    cOutVector packetHopVector;
    cOutVector delayVectorPerNode;
    void registerStats(Packet *pkt);
    void flood(Packet *pkt,int mode);
    void floodLSP(LSP* lsp, int mode);
    LSPRouter* LSPRouter_props;
    simtime_t echo_timer;
    cMessage *helloEvent;
    int echo_count;
    void dijkstra();
    int routing(int dest);
    Dijkstra*  dijkstraModule;
    int* destGate;
public:
    Net();
    virtual ~Net();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
    destGate = (int*)malloc(TOTAL_NODES * sizeof(int));
    for(int i = 0; i<TOTAL_NODES;i++)
        destGate[i] = -1;
}

Net::~Net() {
    free(destGate);
}

void Net::initialize() {
    packetHopVector.setName("packetHops");
    delayVectorPerNode.setName("Delay_per_node");
    helloEvent = new cMessage("helloEvent");
    nodeID = this->getParentModule()->getIndex();

    LSPRouter_props = new LSPRouter(TOTAL_NODES,INTERFACES);
    LSPRouter_props->setRouterID(this->getParentModule()->getIndex());

    echo_count = 0;

    dijkstraModule = new Dijkstra(TOTAL_NODES);


    if(!netGrapthUp){
        Packet *hello = new Packet("hello",4);
        hello->setByteLength(HELLO_BYTE_SIZE);
        hello->setSource(this->getParentModule()->getIndex());
        flood(hello,1);
    }

}

void Net::finish() {
}

void Net::registerStats(Packet *pkt){
    if(pkt->getSource() != this->getParentModule()->getIndex()){
        packetHopVector.record(pkt->getHopCount());
    } else {
        packetHopVector.record(0);
    }

    simtime_t delay = simTime() - pkt->getCreationTime();
    delayVectorPerNode.record(delay);

}

void Net::flood(Packet *pkt, int mode){
    if(mode == 0){ // Normal Flooding
        for(int i = 0;i < gateSize("toLnk$o") ;i++){
            if(this->getParentModule()->gate("toNod$o", i)->isConnected()){
                if(std::strcmp(pkt->getArrivalGate()->getBaseName(),"toLnk") == 0){
                    if(pkt->getArrivalGate()->getIndex() != i){
                        send(pkt->dup(), "toLnk$o", i);
                    }
                } else {
                    send(pkt->dup(), "toLnk$o", i);
                }
            }
        }
    } else if(mode == 1) { //Flooding for packets originated in the router like Hello
        for(int i = 0;i < gateSize("toLnk$o") ;i++){
            if(this->getParentModule()->gate("toNod$o", i)->isConnected())
                send(pkt->dup(), "toLnk$o", i);
        }
    }
}

int Net::routing(int dest) {
    int iterator = INT_MAX;
    int gateOut;
    int neigh = dest;
    int* map = dijkstraModule->getPaths();

    for(int i = 0 ; i< TOTAL_NODES;i++){
        iterator = map[neigh];
        if (iterator == nodeID) break;
        neigh  = iterator;
    }

    for(int i = 0 ; i< INTERFACES;i++)
        if(LSPRouter_props->getNeighList()[i] == neigh){
            gateOut = i;break;
        }
    return gateOut;
}

void Net::floodLSP(LSP *lsp,int mode){
    this->bubble(std::to_string(LSPRouter_props->getLSPCounter()).c_str());
    if(!mode and LSPRouter_props->getLspDB()[lsp->getRouterID()] != NULL)
        return;

    if(LSPRouter_props->isLSPGoodToAdd(lsp)){
        LSPRouter_props->addToLSPDB(lsp);
        int **  neighs = lsp->getNeighCostList();
        for(int i = 0; i < lsp->getMaxNeighs() ;i++)
            if(neighs[i][1] != 0)
                LSPRouter_props->modAdjMatrix(lsp->getRouterID(),neighs[i][0],neighs[i][1]);

        if(LSPRouter_props->getLSPCounter() == TOTAL_NODES){
            cMessage* adjMatrixReady = new cMessage("ready to send",7);
            netGrapthUp = true;
            dijkstraModule->dijkstra(LSPRouter_props->getAdjMatrix(), nodeID);
            send(adjMatrixReady, "toApp$o");
        }
    }

    for(int i = 0;i < gateSize("toLnk$o") ;i++)
        if(this->getParentModule()->gate("toNod$o", i)->isConnected())
            if(mode or std::strcmp(lsp->getArrivalGate()->getBaseName(),"toLnk") == 0)
                if(mode or lsp->getArrivalGate()->getIndex() != i)
                    send(lsp->dup(), "toLnk$o", i);
}
void Net::handleMessage(cMessage *msg) {
    Packet *pkt = (Packet *) msg;

    if(pkt->getKind() > 2){
        pkt->setHopCount(pkt->getHopCount() +1);
        if(pkt->getHopCount() > TOTAL_NODES){
            delete pkt;
            return;
        }
    }

    if(pkt->getKind() == 3){ // Echo
        if(pkt->getSource() != nodeID){
            send(pkt->dup(),"toLnk$o",pkt->getArrivalGate()->getIndex());
        } else {
            simtime_t delay = simTime() - msg->getCreationTime();
            int neighID = LSPRouter_props->getNeighList()[pkt->getArrivalGate()->getIndex()];
            LSPRouter_props->modAdjMatrix(nodeID,neighID, (int) std::ceil(ECHO_WEIGHT_FACTOR*delay.dbl()));
            echo_count++;
//            lsp->setByteLength(LSP_BYTE_SIZE);
            if(echo_count == LSPRouter_props->getNeighCount()){
                LSP *lsp = new LSP(TOTAL_NODES,INTERFACES);
                lsp->setRouterID(nodeID);
                for(int i =0;i < INTERFACES;i++){
                    int* neighs = LSPRouter_props->getNeighList();
                    int** adjMa =  LSPRouter_props->getAdjMatrix();
                    if(neighs[i] != -1){
                        lsp->modNeighCostList(i, neighs[i], adjMa[nodeID][neighs[i]]);
                    }
                }
                LSPRouter_props->addToLSPDB(lsp);
                floodLSP(lsp,1);
            }
        }
    }

    if(pkt->getKind() == 4){ // Hello
        Packet *hello_ack = new Packet("hello_ack",5);
        hello_ack->setByteLength(HELLO_ACK_BYTE_SIZE);
        LSPRouter_props->modNeighList(pkt->getArrivalGate()->getIndex(), pkt->getSource());
        LSPRouter_props->modAdjMatrix(nodeID,pkt->getSource(),1);
        hello_ack->setSource(this->getParentModule()->getIndex());
        send(hello_ack,"toLnk$o",pkt->getArrivalGate()->getIndex());
    }

    if(pkt->getKind() == 5){ // Hello-Ack
        LSPRouter_props->modNeighList(pkt->getArrivalGate()->getIndex(), pkt->getSource());
        LSPRouter_props->modAdjMatrix(nodeID,pkt->getSource(),1);
        LSPRouter_props->addNeighCount();
        Packet *echo = new Packet("echo",3);
        echo->setByteLength(ECHO_BYTE_SIZE);
        echo->setSource(this->getParentModule()->getIndex());
        send(echo,"toLnk$o",pkt->getArrivalGate()->getIndex());
    }

    if(pkt->getKind() == 6){ // LSP
        LSP *lsp = (LSP *) msg;
        floodLSP(lsp,0);
    }

    if(pkt->getKind() > 2){
        delete pkt;
        return;
    }
    // All msg (events) on net are packets
    if(netGrapthUp){
        if(pkt->getKind() == 1){
            pkt->setHopCount(pkt->getHopCount() + 1);
            registerStats(pkt);

            if(pkt->getHopCount() > TOTAL_NODES + 1){
                delete(msg);
                return;
            }
            if (pkt->getDestination() == this->getParentModule()->getIndex()) {
                send(msg, "toApp$o");
            }
            else {
                if(destGate[pkt->getDestination()] == -1)
                    destGate[pkt->getDestination()] = routing(pkt->getDestination());
                send(msg,"toLnk$o",destGate[pkt->getDestination()]);
            }
        }
    }

}
