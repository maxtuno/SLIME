#pragma once

#include "global.hpp"
#include "SimpSolver.h"

class Permutation;
class Specification;

class Breaker {
private:
  std::unordered_set<sptr<Clause>, UVecHash, UvecEqual> clauses;
  std::unordered_set<sptr<PBConstraint>, UVecHash, UvecEqual> pbConstrs;
  sptr<Specification> originalTheory;
  uint nbExtraVars = 0;
  uint nbBinClauses = 0;
  uint nbRowClauses = 0;
  uint nbRegClauses = 0;

  void addUnary(uint l1);
  void addBinary(uint l1, uint l2);
  void addTernary(uint l1, uint l2, uint l3);
  void addQuaternary(uint l1, uint l2, uint l3, uint l4);
  void add(sptr<Clause> cl);
  void add(sptr<Permutation> perm, std::vector<uint>& order, bool limitExtraConstrs);
  void addPB(sptr<Permutation> perm, std::vector<uint>& order, bool limitExtraConstrs);
  void addShatter(sptr<Permutation> perm, std::vector<uint>& order, bool limitExtraConstrs);

public:
  Breaker(sptr<Specification> origTheo);

  ~Breaker() {
  };

  //Prints the current breaker
  void print();

  void addBinClause(uint l1, uint l2);
  void addRegSym(sptr<Permutation> perm, std::vector<uint>& order);
  void addRowSym(sptr<Permutation> perm, std::vector<uint>& order);

  uint getAuxiliaryNbVars();
  uint getTotalNbVars();
  uint getAddedNbClauses();
  uint getTotalNbClauses();

  uint getNbBinClauses();
  uint getNbRowClauses();
  uint getNbRegClauses();

  uint getTseitinVar();

  void to_solver(SLIME::SimpSolver &S);
};
