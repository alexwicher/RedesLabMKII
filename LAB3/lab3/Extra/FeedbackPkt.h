#include <omnetpp.h>

#ifndef FEEDBACKPKT_H
#define FEEDBACKPKT_H

using namespace omnetpp;

class FeedbackPkt: public cMessage {
private:
    int bufferSize_info;
    int pktReceived_info;
public:
    FeedbackPkt(int pkts,int buf);
    void setPktReceived_info(int pkts);
    void setBufferSize_info(int buf);
    int getBufferSize_info();
    int getPktReceived_info();
};

#endif
