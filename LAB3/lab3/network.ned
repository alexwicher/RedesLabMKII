import ned.IUnidirectionalChannel;


simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        volatile int packetByteSize = default(125);
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
        volatile int bufferSize = default(200);
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}


simple TransportTx
{
    parameters:
        volatile int bufferSize = default(200);
        @display("i=block/queue;q=buffer");
    gates:
        input toApp;
        inout toOut;
}


simple TransportRx
{
    parameters:
        volatile int bufferSize = default(200);
        @display("i=block/queue;q=buffer");
    gates:
        output toApp;
        inout toOut;
}


module NodeTx
{
    parameters:
        @display("i=block/routing");
    gates:
        inout toOut;
    submodules:
        gen: Generator {
            parameters:
                @display("p=75,50");
        }
        transTx: TransportTx {
            parameters:
                @display("p=75,190");
        }
    connections:
        gen.out --> transTx.toApp;
        transTx.toOut <--> toOut;
}

module NodeRx
{
    parameters:
        @display("i=block/routing");
    gates:
        inout in;
    submodules:
        sink: Sink {
            parameters:
                @display("p=75,50");
        }
        transRx: TransportRx {
            parameters:
                @display("p=75,190");
        }
    connections:
        in <--> transRx.toOut;
        //        Case Study I
                transRx.toApp --> {datarate = 0.5 Mbps; } --> sink.in;
        //        Case Study II
//       		 transRx.toApp --> {  datarate = 1 Mbps; } --> sink.in;
}

network Network
{
    @display("bgl=2");
    submodules:
        nodeTx: NodeTx {
            @display("p=30,30");
        }
        queue: Queue {
            @display("p=130,30");
        }
        nodeRx: NodeRx {
            @display("p=230,30");
        }
        queue1: Queue {
            @display("p=130,100");
        }
    connections:
        nodeTx.toOut$o --> {  datarate = 1Mbps; delay = 100us; } --> queue.in;
        
        nodeRx.in$o --> {  datarate = 1Mbps; delay = 100us; } --> queue1.in;
        queue1.out --> {  datarate = 1Mbps; delay = 100us; } --> nodeTx.toOut$i;
        //        Case Study I
        		queue.out --> {datarate = 1Mbps;delay = 100us;} --> nodeRx.in$i;
        //        Case Study II
//        queue.out --> {  datarate = 0.5Mbps; delay = 100us; } --> nodeRx.in$i;
}
