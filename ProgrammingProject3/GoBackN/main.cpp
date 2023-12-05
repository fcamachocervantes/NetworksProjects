#include "includes.h"


// ******************************************************************************************
// * Utility functions for the updated version of J. Kurose's simulator.
// * - Student's should need to modify any of this code.
// *
// * Author: Phil Romig, Colorado School of Mines.
// ******************************************************************************************

simulator *simulation;

int main(int argc, char **argv) {

  long nismmax = -1;
  double lossprob = -1.0;
  double corruptprob = - 1.0;
  double lambda = -1.0;
  
  int opt;

  while ((opt = getopt(argc,argv,"n:l:c:t:d:")) != -1) {
    
    switch (opt) {
    case 'n':
      nismmax = std::strtol(optarg,nullptr, 10);
      break;
    case 'l':
      lossprob = std::strtod(optarg, nullptr);
      break;
    case 'c':
      corruptprob = std::strtod(optarg, nullptr);
      break;
    case 't':
      lambda = (float)std::strtod(optarg, nullptr);
      break;
    case 'd':
      LOG_LEVEL = std::strtol(optarg,nullptr, 10);
      break;
    case ':':
    case '?':
    default:
      std::cout << "Usage: " << argv[0]  << " "
        << "-n <messages to simulate> "
        << "-l <prob of loss> "
        << "-c <prob of corruption> "
        << "-t <avg time between messages> "
        << "-d <debug level>" << std::endl;
      std::cout << "\t-d 4 sets log level to info" << std::endl;
      std::cout << "\t-d 5 sets log level to debug" << std::endl;
      std::cout << "\t-d 6 sets log level to trace" << std::endl;
      exit(-1);
    }
  }

  simulation = new simulator(nismmax,lossprob,corruptprob,lambda);

  A_init();
  B_init();
  simulation->go();
}


std::ostream& operator<<(std::ostream& os, const struct msg& message)
{
   for (auto c : message.data) {
    os << c;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const struct pkt& packet)
{
  os << "(seq = " << packet.seqnum;
  os << ", ack = " << packet.acknum;
  os << ", chk = " << packet.checksum << ") ";
  for (auto c : packet.payload) {
    os << c;
  }
  return os;
}
