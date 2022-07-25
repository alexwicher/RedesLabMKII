#include "FeedbackPkt.h"


FeedbackPkt::FeedbackPkt(int pkts,int buf){
    pktReceived_info = pkts;
    bufferSize_info = buf;
}

void FeedbackPkt::setPktReceived_info(int pkts){
    pktReceived_info = pkts;
}

void FeedbackPkt::setBufferSize_info(int buf){
    bufferSize_info = buf;
}

int FeedbackPkt::getPktReceived_info(){
    return pktReceived_info;
}

int FeedbackPkt::getBufferSize_info(){
    return bufferSize_info;
}
