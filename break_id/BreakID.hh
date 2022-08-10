#include <fstream>
#include <sstream>
#include <iterator>
#include <cstring>
#include <stdlib.h>

#include "Graph.hpp"
#include "global.hpp"
#include "Algebraic.hpp"
#include "Theory.hpp"
#include "Breaking.hpp"
#include "SimpSolver.h"

using namespace std;

void setFixedLits(std::string &filename) {
    if (verbosity > 0) {
        std::cout << "c *** Reading fixed variables: " << filename << std::endl;
    }

    ifstream file(filename);
    if (!file) {
        gracefulError("No fixed variables file found.");
    }

    fixedLits.clear();
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        int l;
        while (iss >> l) {
            fixedLits.push_back(encode(l));
            fixedLits.push_back(encode(-l));
        }
    }
}

void addInputSym(sptr<Group> group) {
    if (inputSymFile == "") {
        return; // no input symmetry
    }
    if (verbosity > 0) {
        std::cout << "c *** Reading input symmetry from: " << inputSymFile << std::endl;
    }

    ifstream file(inputSymFile);
    if (!file) {
        gracefulError("No input symmetry file found.");
    }

    string line;
    std::vector<uint> cycle;
    while (getline(file, line)) {
        if (line.front() == '(') { // this is a symmetry generator line
            if (verbosity > 1) {
                std::cout << "c *** adding input symmetry generator" << std::endl;
            }

            sptr<Permutation> perm = std::make_shared<Permutation>();

            stringstream sym(line);
            string cycle_str;

            getline(sym, cycle_str, '('); // skip first empty line
            while (std::getline(sym, cycle_str, '(')) {
                stringstream cycle_ss(cycle_str);
                cycle.clear();
                int l = 0;
                while (cycle_ss >> l) {
                    cycle.push_back(encode(l));
                }
                perm->addCycle(cycle);
            }
            group->add(perm);
            perm->print(std::cerr);
        } else if (line.front() == 'r') { // this is an interchangeability matrix block
            if (verbosity > 1) {
                std::cout << "c *** adding input symmetry matrix" << std::endl;
            }
            uint nbRows;
            uint nbColumns;
            char tmp;
            istringstream iss(line);
            for (uint i = 0; i < 4; ++i) { iss >> tmp; } // skip "rows"
            iss >> nbRows;
            for (uint i = 0; i < 7; ++i) { iss >> tmp; } // skip "columns"
            iss >> nbColumns;
            sptr<Matrix> mat = std::make_shared<Matrix>();

            for (uint r = 0; r < nbRows; ++r) {
                std::vector<uint> *newRow = new std::vector<uint>(nbColumns);
                getline(file, line);
                istringstream iss_row(line);
                int tmp;
                for (uint c = 0; c < nbColumns; ++c) {
                    iss_row >> tmp;
                    (*newRow)[c] = encode(tmp);
                }
                mat->add(newRow);
            }
            group->addMatrix(mat);
        } else {
            gracefulError("Unexpected line in input symmetry file: " + line);
        }
    }
}

namespace options {
    // option strings:
    string instancefile = "-f";
    string nointch = "-no-row";
    string nobinary = "-no-bin";
    string formlength = "-s";
    string verbosity = "-v";
    string timelim = "-t";
    string help = "-h";
    string nosmall = "-no-small";
    string norelaxed = "-no-relaxed";
    string fixedvars = "-fixed";
    string onlybreakers = "-print-only-breakers";
    string generatorfile = "-store-sym";
    string symmetryinput = "-addsym";
    string aspinput = "-asp";
    string pbinput = "-pb";
    string logfile = "-logfile";
}

void printUsage() {
    std::cout << "BreakID version " << VERSION << std::endl;
    std::cout << "Usage: ./BreakID <cnf-file> " <<
              "[" << options::help << "] " <<
              "[" << options::instancefile << " <file with instance>]" <<
              "[" << options::nointch << "] " <<
              "[" << options::nobinary << "] " <<
              "[" << options::nosmall << "] " <<
              "[" << options::norelaxed << "] " <<
              "[" << options::formlength << " <nat>] " <<
              "[" << options::timelim << " <nat>] " <<
              "[" << options::verbosity << " <nat>] " <<
              "[" << options::fixedvars << " <file with fixed vars>] " <<
              "[" << options::onlybreakers << "] " <<
              "[" << options::generatorfile << " <path to store symmetry generators>] " <<
              "[" << options::symmetryinput << "<file with symmetry info>] " <<
              "[" << options::aspinput << "] " <<
              "[" << options::pbinput << " <nat>] " <<
              "[" << options::logfile << " <file to store veriPB log in>]  " <<
              "\n";
    std::cout << "\nOptions:\n";
    std::cout << options::help << "\n  ";
    std::cout << "Display this help message instead of running BreakID.\n";
    std::cout << options::instancefile << "\n";
    std::cout << "Read instance from a file instead of input stream.\n";
    std::cout << options::nointch << "\n  ";
    std::cout << "Disable detection and breaking of row interchangeability.\n";
    std::cout << options::nobinary << "\n  ";
    std::cout << "Disable construction of additional binary symmetry breaking clauses based on stabilizer subgroups.\n";
    std::cout << options::nosmall << "\n  ";
    std::cout << "Disable compact symmetry breaking encoding, use Shatter's encoding instead.\n";
    std::cout << options::norelaxed << "\n  ";
    std::cout << "Disable relaxing constraints on auxiliary encoding variables, use longer encoding instead.\n";
    std::cout << options::formlength << " <default: " << symBreakingFormLength << ">\n  ";
    std::cout << "Limit the size of the constructed symmetry breaking formula's, measured as the number of auxiliary variables introduced. <-1> means no symmetry breaking.\n";
    std::cout << options::timelim << " <default: " << timeLim << ">\n  ";
    std::cout << "Upper limit on time spent by Saucy detecting symmetry measured in seconds.\n";
    std::cout << options::verbosity << " <default: " << verbosity << ">\n  ";
    std::cout << "Verbosity of the output. <0> means no output other than the CNF augmented with symmetry breaking clauses.\n";
    std::cout << options::fixedvars << " <default: none>\n  ";
    std::cout << "File with a list of variables that should be fixed, separated by whitespace.\n";
    std::cout << options::onlybreakers << "\n  ";
    std::cout << "Do not print original theory, only the symmetry breaking clauses.\n";
    std::cout << options::generatorfile << "\n  ";
    std::cout << "Store the detected symmetry generators in the given file.\n";
    std::cout << options::symmetryinput << " <default: none>\n  ";
    std::cout << "Pass a file with symmetry generators or row-interchangeable matrices to use as additional symmetry information. Same format as BreakID's output by "
              << options::generatorfile << ".\n";
    std::cout << options::aspinput << "\n  ";
    std::cout << "Parse input in the smodels-lparse intermediate format instead of DIMACS.\n";
    std::cout << options::pbinput << " <default:unset>\n  ";
    std::cout
            << "When this variable is set, we assume input in the OPB format. The value of the integer indicates the number of variables grouped together in one PB lex-leader constraint. In particular: 1 means almost clausal encoding and infinity means one PB constraint with exponentially sized coefficients. Special case is 0, which takes the PB encoding of symmetry breaking CNF clauses.\n";
    std::cout << options::logfile << "\n  ";
    std::cout << "Store a log in veriPB format in the provided file <default:unset>.\n";
    gracefulError("");
}

void parseOptions(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        string input = argv[i];
        if (0 == input.compare(options::nobinary)) {
            useBinaryClauses = true;
        } else if (0 == input.compare(options::nointch)) {
            useMatrixDetection = true;
        } else if (0 == input.compare(options::onlybreakers)) {
            onlyPrintBreakers = true;
        } else if (0 == input.compare(options::generatorfile)) {
            ++i;
            generatorFile = argv[i];
        } else if (0 == input.compare(options::logfile)) {
            ++i;
            logging = true;
            logstream.open(logfile, ios::out);
            if (logstream.fail()) {
                gracefulError("Could not write to " + logfile);
            }
        } else if (0 == input.compare(options::nosmall)) {
            useShatterTranslation = true;
        } else if (0 == input.compare(options::norelaxed)) {
            useFullTranslation = true;
        } else if (0 == input.compare(options::formlength)) {
            ++i;
            symBreakingFormLength = stoi(argv[i]);
        } else if (0 == input.compare(options::timelim)) {
            ++i;
            timeLim = stoi(argv[i]);
        } else if (0 == input.compare(options::verbosity)) {
            ++i;
            verbosity = stoi(argv[i]);
        } else if (0 == input.compare(options::help)) {
            printUsage();
        } else if (0 == input.compare(options::fixedvars)) {
            ++i;
            string filename = argv[i];
            setFixedLits(filename);
        } else if (0 == input.compare(options::aspinput)) {
            aspinput = true;
        } else if (0 == input.compare(options::pbinput)) {
            ++i;
            nbPBConstraintsToGroup = stoi(argv[i]);
        } else if (0 == input.compare(options::symmetryinput)) {
            ++i;
            inputSymFile = argv[i];
        } else if (0 == input.compare(options::instancefile)) {
            ++i;
            inputFile = argv[i];
        }
    }

    if (verbosity > 1) {
        std::cout << "Options used: " <<
                  options::formlength << " " << symBreakingFormLength << " " <<
                  options::timelim << " " << timeLim << " " <<
                  options::verbosity << " " << verbosity << " " <<
                  (useMatrixDetection ? "" : options::nointch) << " " <<
                  (useBinaryClauses ? "" : options::nobinary) << " " <<
                  (onlyPrintBreakers ? options::onlybreakers : "") << " " <<
                  (generatorFile != "" ? options::generatorfile + " " + generatorFile : " ") << " " <<
                  (logfile != "" ? options::logfile + " " + logfile : " ") << " " <<
                  (inputSymFile != "" ? options::symmetryinput + " " + inputSymFile : " ") << " " <<
                  (inputFile != "" ? options::instancefile + " " + inputFile : " ") << " " <<
                  (useShatterTranslation ? options::nosmall : "") << " " <<
                  (useFullTranslation ? options::norelaxed : "") << " " <<
                  (fixedLits.size() == 0 ? "" : options::fixedvars) << " " << //TODO PRINT AND STORE FIXED LITS FILE NAME
                  (aspinput ? options::aspinput : " ") << " " <<
                  std::endl;
    }
}

// ==== main

void checkOptionsCompatible() {
    if (logging && useBinaryClauses) {
        std::stringstream ss;
        ss << "Please disable binary clause detection when using proof logging. This is not implemented yet. It can be done with " << options::nobinary;
        gracefulError(ss.str());
    }
    if ((aspinput | (nbPBConstraintsToGroup != -1)) && logging) {
        std::stringstream ss;
        ss << "Logging is only implemented with CNF input (not with asp or pb input yet)";
        gracefulError(ss.str());
    }

    if (aspinput && onlyPrintBreakers) {
        std::stringstream ss;
        ss << "Options " << options::aspinput << " and " << options::onlybreakers << " are incompatible since asp output is intertwined with extra clauses.\n";
        gracefulError(ss.str());
    }
}

sptr<Specification> readTheory(std::istream &input) {
    sptr<Specification> theory;
    if (aspinput) {
        theory = make_shared<LogicProgram>(input);
    } else if (nbPBConstraintsToGroup != -1) {
        theory = make_shared<PB>(input);
    } else {
        theory = make_shared<CNF>(input);
    }
    return theory;
}

int break_id_to_slime(const string &file_name, const string &file_proof, SLIME::SimpSolver &S, int slime_rank) {
    time(&startTime);

    inputFile = file_name;
#ifdef LOG
    verbosity = 0;
    mpi_rank = slime_rank;
#endif
    if (!file_proof.empty()) {
        logfile = file_proof;
        logging = true;
        logstream.open(logfile, ios::app);
        if (logstream.fail()) {
            gracefulError("Could not write to " + logfile);
        }
    } else {
        logging = false;
    }

    std::ifstream input(file_name);
    if (input.fail()) {
        gracefulError("No instance file found.");
    }
    sptr<Specification> theory = readTheory(input);

    if (verbosity > 3) {
        theory->getGraph()->print();
    }

    if (verbosity > 0) {
        std::cout << "c *** symmetry generators detected: " << theory->getGroup()->getSize() << std::endl;
        if (verbosity > 2) {
            theory->getGroup()->print(std::cout);
        }
    }

    addInputSym(theory->getGroup());

    if (verbosity > 0) {
        std::cout << "c *** Detecting subgroups..." << std::endl;
    }
    vector<sptr<Group> > subgroups;
    theory->getGroup()->getDisjointGenerators(subgroups);
    if (verbosity > 0) {
        std::cout << "c *** subgroups detected: " << subgroups.size() << std::endl;
    }

    if (verbosity > 1) {
        for (auto grp: subgroups) {
            std::cout << "group size: " << grp->getSize() << " support: " << grp->getSupportSize() << std::endl;
            if (verbosity > 2) {
                grp->print(std::cout);
            }
        }
    }

    theory->cleanUp(); // improve some memory overhead

    uint totalNbMatrices = 0;
    uint totalNbRowSwaps = 0;
    if (logging) {
        nbConstraintsInProofLog = theory->getOriginalSize();
        logstream << "pseudo-Boolean proof version 1.2\n"
                  << "* load formula\n"
                  << "f " << nbConstraintsInProofLog << "\n\n";

    }

    Breaker brkr(theory);
    for (auto grp: subgroups) {
        if (grp->getSize() > 1 && useMatrixDetection) {
            if (verbosity > 1) {
                std::cout << "c *** Detecting row interchangeability..." << std::endl;
            }
            theory->setSubTheory(grp);
            grp->addMatrices();
            totalNbMatrices += grp->getNbMatrices();
            totalNbRowSwaps += grp->getNbRowSwaps();
        }
        if (symBreakingFormLength > -1) {
            if (verbosity > 1) {
                std::cout << "c *** Constructing symmetry breaking formula..." << std::endl;
            }
            grp->addBreakingClausesTo(brkr);
        }// else no symmetry breaking formulas are needed :)
        grp.reset();
    }

    if (verbosity > 0) {
        std::cout << "c *** matrices detected: " << totalNbMatrices << std::endl;
        std::cout << "c *** row swaps detected: " << totalNbRowSwaps << std::endl;
        std::cout << "c *** extra binary symmetry breaking clauses added: " << brkr.getNbBinClauses() << "\n";
        std::cout << "c *** regular symmetry breaking clauses added: " << brkr.getNbRegClauses() << "\n";
        std::cout << "c *** row interchangeability breaking clauses added: " << brkr.getNbRowClauses() << "\n";
        std::cout << "c *** total symmetry breaking clauses added: " << brkr.getAddedNbClauses() << "\n";
        std::cout << "c *** auxiliary variables introduced: " << brkr.getAuxiliaryNbVars() << "\n";
        std::cout << "c *** Printing resulting theory with symmetry breaking clauses..." << std::endl;
    }

    //brkr.print();

    brkr.to_solver(S);

    if (generatorFile != "") {
        if (verbosity > 0) {
            std::cout << "c *** Printing generators to file " + generatorFile << std::endl;
        }
        ofstream fp_out;
        fp_out.open(generatorFile, ios::out);
        if (fp_out.fail()) {
            gracefulError("c Could not write to " + generatorFile);
        }

        for (auto grp: subgroups) {
            grp->print(fp_out);
        }
        fp_out.close();
    }
    if (logging) {
        logstream.close();
    }

    return 0;
}
