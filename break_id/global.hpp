#pragma once

#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <fstream>
#include <functional>
#include <cassert>
#include <time.h>
#include <algorithm>
#include "SimpSolver.h"

#ifndef VERSION
//VERSION will be set when building BreakID with "make"
#define VERSION "unknown-version"
#else
#endif


typedef unsigned int uint;
#define sptr std::shared_ptr

// GLOBALS:
extern uint nVars;
extern std::vector<uint> fixedLits;
extern std::string inputFile;
extern std::string inputSymFile;
extern std::string generatorFile;


extern bool logging;
extern std::ofstream logstream;
extern std::string logfile;
extern uint nbConstraintsInProofLog;


extern time_t startTime;

// OPTIONS:
extern int symBreakingFormLength;
extern bool useBinaryClauses;
extern bool onlyPrintBreakers;
extern bool useMatrixDetection;
extern bool useShatterTranslation;
extern bool useFullTranslation;
extern bool aspinput;
extern int nbPBConstraintsToGroup;
extern uint verbosity;
extern int mpi_rank;
extern int timeLim;

template <class T> size_t _getHash(const std::vector<T>& xs) {
  size_t seed = xs.size();
  for (const T& x : xs) {
    seed ^= ((size_t) x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

int timeLeft();
bool timeLimitPassed();

inline bool sign(uint lit) {
  return lit & 1;
}

inline uint neg(uint lit) {
  return lit ^ 1;
}

inline uint encode(int lit) {
  return (lit > 0 ? 2 * (lit - 1) : 2 * (-lit - 1) + 1);
}

inline int decode(uint lit) {
  return (sign(lit) ? -(lit / 2 + 1) : lit / 2 + 1);
}

void gracefulError(std::string str);

class Clause {
private:
  size_t hashValue;

public:
  std::vector<uint> lits;

  Clause(const std::set<uint>& inLits) :
      hashValue(0), lits(inLits.cbegin(), inLits.cend()) {
  }

  Clause() :
      hashValue(0) {
  }
  ;

  ~Clause() {
  }
  ;

  size_t getHashValue() {
    if (hashValue == 0) {
      hashValue = _getHash(lits);
      if (hashValue == 0) {
        // avoid recalculation of hash
        hashValue = 1;
      }
    }
    return hashValue;
  }

  void print(std::ostream& ostr) {
    for (auto lit : lits) {
      ostr << decode(lit) << " ";
    }
    ostr << "0\n";
  }

  //Prints a clause c a rule falsevar <- (not c)
  //afterwards, falsevar (a new variable) will be added to the "false" constraints.
  void printAsRule(std::ostream& ostr, uint falsevar);

    void to_solver(SLIME::SimpSolver &S);
};


class PBConstraint {
friend class PB;

private:
  size_t hashValue;

  // All terms have a positive coefficient
  // which is possible through the use of literals
  // for a literal and its negation, at most one coefficient (and thus term) exists
  // no 0-coefficient terms exist
  // the addTerm method ensures this is always the case
  std::map<uint,uint> terms; // maps lits to coefficients
  int weight;

public:

  PBConstraint(int w) : hashValue(0), weight(w) {}
  PBConstraint(const Clause& c):hashValue(0), weight(1){
    for(auto l: c.lits) { addTerm(l, 1); }
  }

  /*
   * Add a potentially negated term, which requires adjusting the weight
   * ensure that not both of the lit and the negated lit have a term in the PB
   */
  void addTerm(uint lit, int coeff){
    if(terms.count(lit)!=0){
      coeff += terms[lit];
    }
    if(terms.count(neg(lit))!=0){
      uint c = terms[neg(lit)];
      weight -= c;
      coeff -= terms[neg(lit)];
    }
    if(coeff<0){
      lit = neg(lit);
      coeff = -coeff;
      weight += coeff;
    }
    if(coeff!=0){
      terms[lit]=coeff;
    }else{
      terms.erase(lit);
    }
    terms.erase(neg(lit));
  }

  const std::map<uint,uint>& getTerms() const { return terms; }
  const int getWeight() const { return weight; }

  size_t getHashValue() {
    if (hashValue == 0) {
      hashValue = (size_t) weight;
      for(auto x: terms){
          hashValue ^= ((size_t) x.first) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
          hashValue ^= ((size_t) x.second) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
      }
      if (hashValue == 0) {
        // avoid recalculation of hash
        hashValue = 1;
      }
    }
    return hashValue;
  }

  void print(std::ostream& ostr) {
    int missed = 0;

    bool begin = true;
    for (auto lc : terms) {
      int signedLit = decode(lc.first);
      int coeff = lc.second;
      if(signedLit < 0){
        signedLit = - signedLit;
        missed += coeff;
        coeff = -coeff;
      }
      if(begin){
        begin = false;
      } else{
        if(coeff >0) {
          ostr << "+";
        }
      }
      ostr  << coeff << " x" << signedLit << " ";
    }
    ostr << " >= " << ((int)weight-missed) << "\n";
  }

};

class Rule {
private:
  size_t hashValue;

public:
  //Note: we use "constraints" as basic rules with an empty head. Thus, there is no nice correspondence between our representation of rules and the lparse-smodels format
  //Note: similarly, we use "disjunctive rules" as basic rules with multiple heads. (quite standard representation of those). Again: not nice  correspondence between our representation of rules and the lparse-smodels format
  //to respresent rules. As such, there is currently not yet a GOOD printing method for the rules.
  int ruleType;
  std::vector<uint> headLits;
  std::vector<uint> bodyLits;
  int bound; //Only relevant for rules of type 2 and 5
  std::vector<int> weights;

  Rule(int inRuleType, const std::vector<uint>& inHeads, const std::vector<uint>& bodies, int inBound, std::vector<int>& inWeights) :
      hashValue(0), ruleType(inRuleType), headLits(inHeads.cbegin(), inHeads.cend()), bodyLits(bodies.cbegin(), bodies.cend()), bound(inBound), weights(inWeights.cbegin(),
          inWeights.cend()) {
    std::sort(headLits.begin(), headLits.end());
    if(ruleType != 5 && ruleType != 6){
      std::sort(bodyLits.begin(), bodyLits.end());
    }
    assert(ruleType != 5 || ruleType != 6 || weights.size() == bodyLits.size());
  }

  Rule() :
      hashValue(0), ruleType(1), bound(0) {
  }
  ;

  ~Rule() {
  }
  ;

  size_t getHashValue() {
    if (hashValue == 0) {
      hashValue = ruleType + _getHash(headLits) + 2 * _getHash(bodyLits) + 3 * bound + 4 * _getHash(weights);
      if (hashValue == 0) {
        // avoid recalculation of hash
        hashValue = 1;
      }
    }
    return hashValue;
  }

  void print(std::ostream& ostr) {
    std::cerr << "Not implemented: printing rules\n";
    /*std::cerr << "WARNING: printing rules is NOT in the right order for lparse format\n";
     if (basicRule && headLits.size() == 0) {
     //A constraint
     assert(bodyLits.size() == 1);
     auto origLit = decode(bodyLits.front());
     if (origLit > 0) {
     ostr << "B-\n" << origLit;
     } else {
     ostr << "B+\n" << origLit;
     }
     } else {
     if (basicRule) {
     assert(headLits.size() == 1);
     ostr << "1 " << decode(headLits.front()) << " ";
     } else {
     ostr << "3 ";
     ostr << headLits.size() << " ";
     for (auto lit : headLits) {
     ostr << decode(lit) << " ";
     }
     }
     std::set<uint> posBodyLits;
     std::set<uint> negBodyLits;
     for (auto lit : bodyLits) {
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
     }
     ostr << std::endl;*/
  }
};

struct UVecHash {

  size_t operator()(const sptr<std::vector<uint> > first) const {
    return _getHash(*first);
  }

  size_t operator()(const sptr<Clause> cl) const {
    return cl->getHashValue();
  }

  size_t operator()(const sptr<Rule> r) const {
    return r->getHashValue();
  }

  size_t operator()(const sptr<PBConstraint> r) const {
    return r->getHashValue();
  }
};

struct UvecEqual {
  template <class T>
  bool equals(const std::vector<T>& first, const std::vector<T>& second) const {
    if (first.size() != second.size()) {
      return false;
    }
    for (unsigned int k = 0; k < first.size(); ++k) {
      if (first.at(k) != second.at(k)) {
        return false;
      }
    }
    return true;
  }

  template <class T>
  bool operator()(const sptr<std::vector<T> > first, const sptr<std::vector<T> > second) const {
    return equals(*first, *second);
  }

  bool operator()(const sptr<Clause> first, const sptr<Clause > second) const {
    return equals(first->lits, second->lits);
  }

  bool operator()(const sptr<Rule> first, const sptr<Rule > second) const {
    return (first->ruleType == second->ruleType) && equals(first->headLits, second->headLits) && equals(first->bodyLits, second->bodyLits) && equals(first->weights, second->weights) && (first->bound == second->bound);
  }

  bool operator()(const sptr<PBConstraint> first, const sptr<PBConstraint > second) const {
    auto& lhs = first->getTerms();
    auto& rhs = second->getTerms();
    return first->getWeight()==second->getWeight() && lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(),rhs.begin());
  }
};

template<class T>
bool isDisjoint(std::unordered_set<T>& uset, std::vector<T>& vec) {
  for (T x : vec) {
    if (uset.count(x) > 0) {
      return false;
    }
  }
  return true;
}

template<class T>
void swapErase(std::vector<T>& vec, uint index) {
  vec[index] = vec.back();
  vec.pop_back();
}
