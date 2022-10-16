#include <fstream>
#include <stdlib.h>
#include "global.hpp"
#include "limits.h"
#include "SimpSolver.h"

using namespace std;

uint nVars = 0;
std::vector<uint> fixedLits;
std::string inputFile = "";
std::string inputSymFile = "";
std::string generatorFile = "";
std::string logfile = "";
std::ofstream logstream;

time_t startTime;


// OPTIONS:
bool logging = true;
bool useMatrixDetection = false;
bool useBinaryClauses = true;
bool onlyPrintBreakers = false;
bool useShatterTranslation = true;
bool useFullTranslation = true;
int symBreakingFormLength = INT_MAX;
bool aspinput = false;
int nbPBConstraintsToGroup = -1;
uint verbosity = 0;
int mpi_rank = 0;
int timeLim = INT_MAX;
uint nbConstraintsInProofLog = 0;

int timeLeft() {
  time_t now;
  time(&now);
  return timeLim - difftime(now, startTime);
}

bool timeLimitPassed() {
  return timeLeft() <= 0;
}

void gracefulError(string str) {
  std::cerr << str << "\nExiting..." << endl;
  exit(1);
}

void Clause::to_solver(SLIME::SimpSolver &S) {
    SLIME::vec<SLIME::Lit> ls;
    for (auto item : lits) {
        int parsed_lit = decode(item);
        if (parsed_lit == 0)
            break;
        int var = abs(parsed_lit) - 1;
        while (var >= S.nVars())
            S.newVar();
        ls.push((parsed_lit > 0) ? SLIME::mkLit(var) : ~SLIME::mkLit(var));
    }
    S.addClause_(ls);
}

//Prints a clause c a rule falsevar <- (not c)
//afterwards, falsevar (a new variable) will be added to the "false" constraints.
void Clause::printAsRule(std::ostream& ostr, uint falsevar) {
  ostr << "1 " << falsevar << " ";
  std::set<uint> posBodyLits;
  std::set<uint> negBodyLits;
  for (auto lit : lits) {
    auto decoded = decode(lit);
    if (decoded > 0) {
      posBodyLits.insert(decoded);
    } else {
      negBodyLits.insert(-decoded);
    }
  }
  ostr << (posBodyLits.size() + negBodyLits.size()) << " " << negBodyLits.size() << " ";
  for (auto decodedlit : negBodyLits) {
    ostr << decodedlit << " ";
  }
  for (auto decodedlit : posBodyLits) {
    ostr << decodedlit << " ";
  }
  ostr << std::endl;
}