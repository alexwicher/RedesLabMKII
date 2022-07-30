#ifndef TRANSPORTTX
#define TRANSPORTTX

#include <string.h>
#include <omnetpp.h>

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

public:
    TransportTx();

    virtual ~TransportTx();

protected:
    virtual void initialize();

    virtual void finish();

    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportTx);


TransportTx::TransportTx() {
    endServiceEvent = NULL;
}

TransportTx::~TransportTx() {
    cancelAndDelete(endServiceEvent);
}

void TransportTx::initialize() {

    bufferSizeVector.setName("bufferSize");
    packetDropVector.setName("packetDrop");
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endService");
}

void TransportTx::finish() {
}

void TransportTx::handleMessage(cMessage *msg) {

if (msg->getKind() == 0) {
        if (msg == endServiceEvent) {
            if (!buffer.isEmpty()) {
                cPacket *pkt = (cPacket *) buffer.pop();
                send(pkt, "toOut$o");
                simtime_t serviceTime = pkt->getDuration();
                scheduleAt(simTime() + serviceTime, endServiceEvent);
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
