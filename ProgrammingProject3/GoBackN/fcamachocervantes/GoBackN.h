
// ***********************************************************
// * Any additional include files should be added here.
// ***********************************************************

// ***********************************************************
// * Any functions you want to add should be included here.
// ***********************************************************
const int WINDOW_SIZE = 10;
const double ALPHA = 0.125;

struct pkt make_pkt(int sequenceNumber, char data[20]);
int computeChecksum(struct pkt packet);