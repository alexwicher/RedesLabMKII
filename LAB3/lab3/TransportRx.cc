#ifndef TRANSPORTRX
#define TRANSPORTRX

#include <string.h>
#include <omnetpp.h>
#include <Extra/FeedbackPkt.h>

#define EMERGENCY_PERCENT_BUFF 96
#define STATUS_PKT_TIMER 0.6 // seconds

using namespace omnetpp;

class TransportRx : public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    simtime_t control_pkt_timer;
    int received_pkts = 0;
public:
    TransportRx();

    virtual ~TransportRx();

protected:
    virtual void initialize();

    virtual void finish();

    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportRx);

TransportRx::TransportRx() {
    endServiceEvent = NULL;
}

TransportRx::~TransportRx() {
    cancelAndDelete(endServiceEvent);
}

void TransportRx::initialize() {
    control_pkt_timer = simTime();
    bufferSizeVector.setName("bufferSize");
    packetDropVector.setName("packetDrop");
    buffer.setName("buffer");
    endServiceEvent = new cMessage("endService");
}

void TransportRx::finish() {
}

void TransportRx::handleMessage(cMessage *msg) {
    if (msg->getKind() == 0) {
        if (msg == endServiceEvent) {
            if (!buffer.isEmpty()) {
                cPacket *pkt = (cPacket *) buffer.pop();
                send(pkt, "toApp");
                simtime_t serviceTime = pkt->getDuration();
                scheduleAt(simTime() + serviceTime, endServiceEvent);
            }
        } else {
            if (buffer.getLength() >= (long) par("bufferSize")) {
                delete msg;
                this->bubble("packet dropped");
                packetDropVector.record(1);
            } else {
                received_pkts++;
                buffer.insert(msg);
                bufferSizeVector.record(buffer.getLength());
                if (!endServiceEvent->isScheduled()) {
                    scheduleAt(simTime() + 0, endServiceEvent);
                }
            }

        }
    }
    FeedbackPkt *emergencyPkt = new FeedbackPkt(received_pkts, buffer.getLength());
    if ((buffer.getLength() * 100) / ((long) par("bufferSize")) > (long) EMERGENCY_PERCENT_BUFF) {
        this->bubble("Emergency flow sent");
        emergencyPkt->setKind(1);
        send(emergencyPkt, "toOut$o");
    } else if (simTime() - control_pkt_timer > STATUS_PKT_TIMER) {
        this->bubble("Control pkt sent");
        emergencyPkt->setKind(2);
        send(emergencyPkt, "toOut$o");
        control_pkt_timer = simTime();
    }


}

#endif
