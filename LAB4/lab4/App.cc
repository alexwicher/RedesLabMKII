#ifndef APP
#define APP

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

using namespace omnetpp;

class App: public cSimpleModule {
private:
    cMessage *sendMsgEvent;
    cStdDev delayStats;
    cOutVector delayVector;
    bool networkUp;
    cOutVector pktsReceivedVector;
    cStdDev pktsReceivedStats;

public:
    App();
    virtual ~App();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(App);

#endif /* APP */

App::App() {
}

App::~App() {
}

void App::initialize() {
    // Initialize statistics
    networkUp = false;
    delayStats.setName("TotalDelay");
    delayVector.setName("Delay_destine_node");
    pktsReceivedVector.setName("pkts_received");
    pktsReceivedStats.setName("TotalPktsReceived");
}

void App::finish() {
    // Record statistics
    recordScalar("Average delay", delayStats.getMean());
    recordScalar("Number of packets", delayStats.getCount());
}

void App::handleMessage(cMessage *msg) {

    if(msg->getKind() == 7 and par("interArrivalTime").doubleValue() != 0){ // Network ready
        networkUp = true;
        sendMsgEvent = new cMessage("sendEvent");
        scheduleAt(par("interArrivalTime") + simTime(), sendMsgEvent);
        return;
    }

        if (msg == sendMsgEvent and networkUp) {
            // create new packet
            Packet *pkt = new Packet("data",1);
            pkt->setByteLength(par("packetByteSize"));
            pkt->setSource(this->getParentModule()->getIndex());
            pkt->setDestination(par("destination"));

            // send to net layer
            send(pkt, "toNet$o");

            // compute the new departure time and schedule next sendMsgEvent
            simtime_t departureTime = simTime() + par("interArrivalTime");
            scheduleAt(departureTime, sendMsgEvent);

        }
        // else, msg is a packet from net layer
        else {
            // compute delay and record statistics
            simtime_t delay = simTime() - msg->getCreationTime();
            delayStats.collect(delay);
            delayVector.record(delay);

            pktsReceivedVector.record(1);
            pktsReceivedStats.collect(1);
            // delete msg
            delete (msg);
        }

}
