#ifndef TRANSPORTTX
#define TRANSPORTTX

#include <string.h>
#include <omnetpp.h>
#include "Extra/FeedbackPkt.h"

#define FLOW_TIMER 10 // seconds
#define FLOW_DELAY_RATE 0.025 // seconds
#define MIN_FAILSAFE_BUFFERSIZE_FLOW 45

#define CONGESTION_DELAY_RATE 0.025 // seconds
#define CONGESTION_PKTS_DIFF_TOLERANCE 85 //Ideally proportional to minimum buffer share in sub-net
#define CONGESTION_TIMER 10


using namespace omnetpp;

class TransportTx : public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;

    simtime_t flow_delay;
    simtime_t congestion_delay;
    simtime_t aux_flow_timer;
    simtime_t aux_congestion_timer;
    int sent;

    void reSetTimerFlow();
    void reSetTimerCongestion();
    void hardResetFlowDelayFailsafe(FeedbackPkt *flowPkt);
public:
    TransportTx();

    virtual ~TransportTx();

protected:
    virtual void initialize();

    virtual void finish();

    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportTx);


void TransportTx::reSetTimerFlow(){
    if (aux_flow_timer != 0 and simTime() - aux_flow_timer > FLOW_TIMER) {
        this->bubble("Reseting flow delay.");
        flow_delay = 0;
    }
}

void TransportTx::reSetTimerCongestion(){
    if(aux_congestion_timer != 0 and simTime() - aux_congestion_timer > CONGESTION_TIMER){
        this->bubble("Reseting congestion delay.");
        congestion_delay = 0;
    }
}

void TransportTx::hardResetFlowDelayFailsafe(FeedbackPkt *flowPkt){
    if(flow_delay != 0 and flowPkt->getBufferSize_info() < MIN_FAILSAFE_BUFFERSIZE_FLOW){
        this->bubble("Hard resetting flow delay.");
        flow_delay = 0;
        aux_flow_timer = 0;
    }
}

TransportTx::TransportTx() {
    endServiceEvent = NULL;
}

TransportTx::~TransportTx() {
    cancelAndDelete(endServiceEvent);
}

void TransportTx::initialize() {

    flow_delay = 0;
    congestion_delay = 0;
    sent = 0;

    aux_flow_timer = 0;
    aux_congestion_timer = 0;

    bufferSizeVector.setName("bufferSize");
    packetDropVector.setName("packetDrop");
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endService");
}

void TransportTx::finish() {
}

void TransportTx::handleMessage(cMessage *msg) {

    if (msg->getKind() == 1) {
        FeedbackPkt *flowPkt = (FeedbackPkt *) msg;
        this->bubble("Emergency flow received");
        flow_delay += FLOW_DELAY_RATE;
        aux_flow_timer = simTime();
        delete (msg);
    } else if (msg->getKind() == 2) {
        FeedbackPkt *flowPkt = (FeedbackPkt *) msg;
        double recvSentDiff = sent - flowPkt->getPktReceived_info();
        const char *ReceptorInf = (/*"S: " + std::to_string(sent) + " R: " + std::to_string(flowPkt->getPktReceived_info())*/
                + " buff: " + std::to_string(flowPkt->getBufferSize_info())).c_str();
        this->bubble(ReceptorInf);

        hardResetFlowDelayFailsafe(flowPkt);

        if (recvSentDiff > CONGESTION_PKTS_DIFF_TOLERANCE) {
            this->bubble("Slowing down, possible congestion");
            aux_congestion_timer = simTime();
            congestion_delay += CONGESTION_DELAY_RATE;
        }
    } else if (msg->getKind() == 0) {
        reSetTimerCongestion();
        reSetTimerFlow();
        if (msg == endServiceEvent) {
            if (!buffer.isEmpty()) {
                cPacket *pkt = (cPacket *) buffer.pop();
                send(pkt, "toOut$o");
                sent++;
                simtime_t serviceTime = pkt->getDuration();
                scheduleAt(simTime() + serviceTime + flow_delay + congestion_delay, endServiceEvent);
            }
        } else {
            if (buffer.getLength() >= (long) par("bufferSize")) {
                delete msg;
                this->bubble("packet dropped");
                packetDropVector.record(1);
            } else {
                buffer.insert(msg);
                bufferSizeVector.record(buffer.getLength());
                if (!endServiceEvent->isScheduled()) {
                    scheduleAt(simTime() + 0, endServiceEvent);
                }
            }

        }
    }
}

#endif
