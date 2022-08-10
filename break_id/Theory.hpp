#pragma once

#include "global.hpp"
#include "SimpSolver.h"

class Graph;
class Permutation;
class Group;
class Matrix;
class Breaker;

class Specification{
  friend class Breaker;
protected:
  sptr<Graph> graph;
  sptr<Group> group;
  std::vector<std::string> originalSpec;
public:
  Specification();
  virtual ~Specification();

  virtual void print(std::ostream& out)=0;
  virtual uint getSize()=0;
  virtual uint getOriginalSize()=0;

  sptr<Graph> getGraph();
  sptr<Group> getGroup();

  virtual void cleanUp();

  virtual void setSubTheory(sptr<Group> subgroup)=0;

  virtual bool isSymmetry(Permutation& prm)=0;

  virtual void to_solver(SLIME::SimpSolver &S)=0;
};

class CNF: public Specification {
  friend class Breaker;

private:
  std::unordered_set<sptr<Clause>, UVecHash, UvecEqual> clauses; // must be an unordered_set, since we need to be able to test whether a clause exists to detect symmetries

  void readCNF(std::istream& input);
  uint origNbClauses;

public:
  CNF(std::istream& input);
  CNF(std::vector<sptr<Clause> >& clss, sptr<Group> grp);
  ~CNF();



  void print(std::ostream& out);
  uint getSize();
  uint getOriginalSize();

  void setSubTheory(sptr<Group> subgroup);

  bool isSymmetry(Permutation& prm);


  void to_solver(SLIME::SimpSolver &S);
};


class LogicProgram: public Specification  {
  friend class Breaker;

private:
  std::unordered_set<sptr<Rule>, UVecHash, UvecEqual> rules; // must be an unordered_set, since we need to be able to test whether a rule exists to detect symmetries
  void readLogicProgram(std::istream& input);

public:
  LogicProgram(std::istream& input);
  LogicProgram(std::vector<sptr<Rule> >& rls, sptr<Group> grp);
  ~LogicProgram();

  void print(std::ostream& out);

  uint getSize();
  uint getOriginalSize();


  void setSubTheory(sptr<Group> subgroup);

  bool isSymmetry(Permutation& prm);

    void to_solver(SLIME::SimpSolver &S) override;
};



class PB: public Specification  {
  friend class Breaker;

private:
  std::unordered_set<sptr<PBConstraint>, UVecHash, UvecEqual> constraints; // must be an unordered_set, since we need to be able to test whether a rule exists to detect symmetries
  void readPB(std::istream& input);
  //Takes as input a vector of variables (not lits) and a vector of (possibly neg) coefficients and a weight. Transforms into a pbconstraint and adds.
  void processInequality(std::vector<int> lits, std::vector<int> coefs, int w);

public:
  PB(std::istream& input);
  PB(std::vector<sptr<PBConstraint> >& rls, sptr<Group> grp);

  void print(std::ostream& out);

  uint getSize();
  uint getOriginalSize();


  void setSubTheory(sptr<Group> subgroup);

  bool isSymmetry(Permutation& prm);

    void to_solver(SLIME::SimpSolver &S) override;
};


