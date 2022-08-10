#include "Theory.hpp"
#include <fstream>
#include <sstream>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <bitset>
#include "Graph.hpp"
#include "Algebraic.hpp"
#include "Breaking.hpp"

//=========================CNF==================================================

using namespace std;

void CNF::readCNF(std::istream& input) {
  string line;
  std::set<uint> inclause = std::set<uint>();
  while (getline(input, line)) {
    if (line.size() == 0 || line.front() == 'c') {
      // do nothing, this is a comment line
    } else if (line.front() == 'p') {
      string info = line.substr(6);
      istringstream iss(info);
      uint nbVars;
      iss >> nbVars;
      uint nbClauses;
      iss >> nbClauses;
      origNbClauses = nbClauses;
      if (verbosity > 1) {
        std::cout << "CNF header stated " << nbVars << " vars and " << nbClauses << " clauses" << std::endl;
      }
      nVars = nbVars;
      clauses.reserve(nbClauses);
    } else {
      //  parse the clauses, removing tautologies and double lits
      istringstream iss(line);
      int l;
      while (iss >> l) {
        if (l == 0) {
          if (inclause.size() == 0) {
            gracefulError("Theory can not contain empty clause.");
          }
          bool isTautology = false;
          for (auto lit : inclause) {
            if (inclause.count(neg(lit)) > 0) {
              isTautology = true;
              break;
            }
          }
          if (not isTautology) {
            sptr<Clause> cl(new Clause(inclause));
            clauses.insert(cl);
          }
          inclause.clear();
        } else {
          if ((uint) abs(l) > nVars) {
            nVars = abs(l);
          }
          inclause.insert(encode(l));
        }
      }
    }
  }
}

CNF::CNF(std::istream& input) {
  readCNF(input);
  if (verbosity > 0) {
    std::cout << "c *** Creating first graph..." << std::endl;
  }
  graph = make_shared<Graph>(clauses);
  if(verbosity > 1){
    std::cout << "c **** Number of nodes: " << graph->getNbNodes() << std::endl;
    std::cout << "c **** Number of edges: " << graph->getNbEdges() << std::endl;
  }

  group = make_shared<Group>();
  if (verbosity > 0) {
    std::cout << "c *** Detecting symmetry group..." << std::endl;
  }
  std::vector<sptr<Permutation> > symgens;
  graph->getSymmetryGenerators(symgens);
  for (auto symgen : symgens) {
    group->add(symgen);
  }
}

CNF::CNF(std::vector<sptr<Clause> >& clss, sptr<Group> grp) {
  clauses.insert(clss.cbegin(), clss.cend());
  graph = make_shared<Graph>(clauses);
  group = grp;
  for (uint l = 0; l < 2 * nVars; ++l) {
    if (not grp->permutes(l)) {
      graph->setUniqueColor(l);
    }
  }
  for(uint m=0; m<grp->getNbMatrices(); ++m){
    auto mat = grp->getMatrix(m);
    for(uint r=0; r<mat->nbRows()-1; ++r){
      getGraph()->setUniqueColor(*(mat->getRow(r)));
    }
  }
}

CNF::~CNF() {
}

void CNF::to_solver(SLIME::SimpSolver& S) {
    for (auto clause : clauses) {
        clause->to_solver(S);
    }
}

void CNF::print(std::ostream& out) {
  for (auto clause : clauses) {
    clause->print(out);
  }
}

uint CNF::getSize() {
  return clauses.size();
}

uint CNF::getOriginalSize() {
  return origNbClauses;
}

void CNF::setSubTheory(sptr<Group> subgroup) {
  //TODO: what is this method supposed to do: keep all clauses that are not mapped to themselves? Is it simply made approximative on purpose or by accident?
  std::vector<sptr<Clause> > subclauses;
  for (auto cl : clauses) {
    for (auto lit : cl->lits) {
      if (subgroup->permutes(lit)) {
        subclauses.push_back(cl);
        break;
      }
    }
  }
  subgroup->theory = new CNF(subclauses, subgroup);
}

bool CNF::isSymmetry(Permutation& prm) {
  for (auto cl : clauses) {
    sptr<Clause> symmetrical(new Clause());
    if (!prm.getImage(cl->lits, symmetrical->lits)) {
      continue;
    }
    std::sort(symmetrical->lits.begin(), symmetrical->lits.end());
    if (clauses.count(symmetrical) == 0) {
      return false;
    }
  }
  return true;
}

/******************
 * SPECIFICATION
 *
 */

Specification::~Specification() {

}

Specification::Specification() {

}

sptr<Graph> Specification::getGraph() {
  return graph;
}

sptr<Group> Specification::getGroup() {
  return group;
}

void Specification::cleanUp() {
  graph.reset();
  group.reset();
}

/******************
 * LOGIC PROGRAM
 */

void checkVarExists(int lit) {
  if ((uint) abs(lit) > nVars) {
    nVars = abs(lit);
  }
}
void LogicProgram::readLogicProgram(std::istream& input) {
  string line;
  vector<uint> headlits;
  vector<uint> bodylits;
  vector<int> weights;

  //PARSE RULES:
  if (verbosity > 5) {
    std::cout << "c ***** Reading RULES: " << std::endl;
  }
  while (getline(input, line)) {
    originalSpec.push_back(line);
    if (line.size() == 0 || line.front() == '%') {
      continue;
    }
    if (line.front() == '0') {
      break;
    }
    istringstream iss(line);

    int bound = 0;
    int rule_type;
    iss >> rule_type;
    if (rule_type != 1 && rule_type != 2 && rule_type != 3 && rule_type != 5 && rule_type != 6 && rule_type != 8) {
      std::cerr << "UNSUPPORTED RULE: " << line << std::endl;
      gracefulError(
          "Unsupported rule type: currently only supporting basic(1), constraint(2), choice(3), weight(5), minimize(6) and disjunctive(8) rules. Use lp2normal to use this tool with a richer language");
      //TODO: Optimisation rules: fixed literals.
    }

    //Parse head of rule
    if (rule_type == 1 || rule_type == 2 || rule_type == 5) {
      int head;
      iss >> head;
      checkVarExists(head);
      headlits.push_back(encode(head));
    } else if (rule_type == 3 || rule_type == 8) {
      int nbheads;
      iss >> nbheads;
      int lit;
      for (int i = 0; i < nbheads; i++) {
        iss >> lit;
        checkVarExists(lit);
        headlits.push_back(encode(lit));
      }
    } else if (rule_type == 6) {
      int lit;
      iss >> lit;
      assert(lit == 0);
    }

    if (rule_type == 5) {
      iss >> bound;
    }

    //Parse body of rule
    int nblits;
    int nbneglits;
    iss >> nblits;
    iss >> nbneglits;

    if (rule_type == 2) {
      iss >> bound;
    }

    int i = 0;
    int lit;
    for (; i < nbneglits; i++) {
      iss >> lit;
      checkVarExists(lit);
      bodylits.push_back(encode(-lit));
    }
    for (; i < nblits; i++) {
      iss >> lit;
      checkVarExists(lit);
      bodylits.push_back(encode(lit));
    }

    //parse weights
    if (rule_type == 5 || rule_type == 6) {
      int weight;
      for (i=0; i < nblits; i++) {
        iss >> weight;
        weights.push_back(weight);
      }
    }

    sptr<Rule> r(new Rule(rule_type, headlits,bodylits, bound, weights));
    rules.insert(r);
    headlits.clear();
    bodylits.clear();
    weights.clear();
  }

  //PARSE SYMBOL TABLE:
  if (verbosity > 5) {
    std::cout << "c ***** Reading SYMBOL TABLE: " << std::endl;
  }
  while (getline(input, line)) {
    originalSpec.push_back(line);
    if (line.size() == 0 || line.front() == '%') {
      continue;
    }
    if (line.front() == '0') {
      break;
    }
    //Currently not storing this information
  }

  //PARSE CONSTRAINTS:
  if (verbosity > 5) {
    std::cout << "c ***** Reading CONSTRAINTS: " << std::endl;
  }
  bool negative = false;
  while (getline(input, line)) {
    originalSpec.push_back(line);
    if (line.size() == 0 || line.front() == '%') {
      continue;
    }
    if (line.front() == '0') {
      if (negative) {
        break;
      } else {
        continue;
      }
    }
    if (line.front() == 'B') {
      if (line.at(1) == '+') {
        negative = false;
        continue;
      }
      if (line.at(1) == '-') {
        negative = true;
        continue;
      }
    }
    int lit;
    istringstream iss(line);
    iss >> lit;
    if (negative) {
      //Note the negation caused by putting this in the body of a rule: negative constraint is positive body.
      bodylits.push_back(encode(lit));
    } else {
      bodylits.push_back(encode(-lit));
    }

    //Constraints are basic rules with empty head
    sptr<Rule> r(new Rule(1, headlits, bodylits, 0, weights));
    rules.insert(r);
    headlits.clear();
    bodylits.clear();
  }

}
LogicProgram::LogicProgram(std::istream& input) {
  readLogicProgram(input);
  if (verbosity > 6) {
    std::cout << "Parsed logic program:\n";
    print(std::cout);
  }
  if (verbosity > 0) {
    std::cout << "c *** Creating first graph..." << std::endl;
  }
  graph = make_shared<Graph>(rules);
  if(verbosity > 1){
    std::cout << "c **** Number of nodes: " << graph->getNbNodes() << std::endl;
    std::cout << "c **** Number of edges: " << graph->getNbEdges() << std::endl;
  }

  group = make_shared<Group>();
  if (verbosity > 0) {
    std::cout << "c *** Detecting symmetry group..." << std::endl;
  }
  std::vector<sptr<Permutation> > symgens;
  graph->getSymmetryGenerators(symgens);
  for (auto symgen : symgens) {
    group->add(symgen);
  }
}

LogicProgram::LogicProgram(std::vector<sptr<Rule> >& rls, sptr<Group> grp) {
  rules.insert(rls.cbegin(), rls.cend());
  graph = make_shared<Graph>(rules);
  group = grp;
  for (uint l = 0; l < 2 * nVars; ++l) {
    if (not grp->permutes(l)) {
      graph->setUniqueColor(l);
    }
  }
  for(uint m=0; m<grp->getNbMatrices(); ++m){
    auto mat = grp->getMatrix(m);
    for(uint r=0; r<mat->nbRows()-1; ++r){
      getGraph()->setUniqueColor(*(mat->getRow(r)));
    }
  }
}

//CNF(std::vector<sptr<Clause> >& clss, sptr<Group> grp);
LogicProgram::~LogicProgram() {

}

void LogicProgram::print(std::ostream& out) {
  for (auto rule : rules) {
    rule->print(out);
  }
}

uint LogicProgram::getSize() {
  return rules.size();

}
uint LogicProgram::getOriginalSize(){
  return getSize(); //TODO CAN THIS DIFFER?
}


void LogicProgram::setSubTheory(sptr<Group> subgroup) {
  //TODO: what is this method supposed to do: keep all clauses that are not mapped to themselves? Is it simply made approximative on purpose or by accident?
  std::vector<sptr<Rule> > subrules;
  for (auto r : rules) {
    bool found = false;
    for (auto lit : r->headLits) {
      if(found) {
        break;
      }
      if (subgroup->permutes(lit)) {
        subrules.push_back(r);
        found = true;
      }
    }
    for (auto lit : r->bodyLits) {
      if(found) {
        break;
      }
      if (subgroup->permutes(lit)) {
        subrules.push_back(r);
        found = true;
      }
    }
  }
  subgroup->theory = new LogicProgram(subrules, subgroup);
}

bool LogicProgram::isSymmetry(Permutation &prm) {
  for (auto r : rules) {
    sptr<Rule> symmetrical(new Rule());
    if ( ! (prm.getImage(r->headLits, symmetrical->headLits) | prm.getImage(r->bodyLits, symmetrical->bodyLits) )) {
      continue;
    }
    symmetrical->weights = r->weights;
    symmetrical->ruleType = r->ruleType;
    symmetrical->bound = r->bound;
    std::sort(symmetrical->headLits.begin(), symmetrical->headLits.end());
    if(r->ruleType != 5 && r->ruleType != 6) {
      std::sort(symmetrical->bodyLits.begin(), symmetrical->bodyLits.end());
    }
    if (rules.count(symmetrical) == 0) {
      return false;
    }
  }
  return true;
}

void LogicProgram::to_solver(SLIME::SimpSolver &S) {

};


/********************PB*/

void PB::processInequality(vector<int> lits, vector<int> coefs, int w){
  auto constr = make_shared<PBConstraint>(w);
  for(size_t i=0;i<lits.size();i++){
    constr->addTerm(encode(lits[i]),coefs[i]);
  }

  if(constr->getWeight() <= 0){ return; }//already satisfied.
  constraints.insert(constr);
}

void PB::readPB(std::istream& input) {
  for (string line; getline(input, line);) {
    if (line.empty()) continue;
    else if (line[0] == '*') {
      if (line.substr(0, 13) == "* #variable= ") {
        istringstream is (line.substr(13));
        is >> nVars;
      }
    } else {
      string symbol;
      if (line.find(">=") != string::npos) symbol = ">=";
      else symbol = "=";
      assert(line.find(symbol) != string::npos);
      istringstream is (line.substr(0, line.find(symbol)));
      vector<int> lits;
      vector<int> coefs;
      int coef;
      string var;
      while (is >> coef >> var) {
        assert(coef != 0 && abs(coef) <= (int) 1e9);
        int x = atoi(var.substr(1).c_str());
        lits.push_back(x);
        coefs.push_back(coef);
      }
      int w = atoi(line.substr(line.find("=") + 1).c_str());
      processInequality(lits, coefs, w);
      // Handle equality case with two constraints
      if (line.find(" = ") != string::npos) {
        for (int & coef : coefs) coef = -coef;
        w *= -1;
        processInequality(lits, coefs, w);
      }
    }
  }
}


PB::PB(std::istream& input){
  readPB(input);
  if (verbosity > 6) {
    std::cout << "Parsed PB theory:\n";
    print(std::cout);
  }
  if (verbosity > 0) {
    std::cout << "c *** Creating first graph..." << std::endl;
  }
  graph = make_shared<Graph>(constraints);
  if(verbosity > 1){
    std::cout << "c **** Number of nodes: " << graph->getNbNodes() << std::endl;
    std::cout << "c **** Number of edges: " << graph->getNbEdges() << std::endl;
  }

  group = make_shared<Group>();
  if (verbosity > 0) {
    std::cout << "c *** Detecting symmetry group..." << std::endl;
  }
  std::vector<sptr<Permutation> > symgens;
  graph->getSymmetryGenerators(symgens);
  for (auto symgen : symgens) {
    group->add(symgen);
  }

}

PB::PB(std::vector<sptr<PBConstraint> > &constr, sptr<Group> grp) {
  constraints.insert(constr.cbegin(), constr.cend());
  graph = make_shared<Graph>(constraints);
  group = grp;
  for (uint l = 0; l < 2 * nVars; ++l) {
    if (not grp->permutes(l)) {
      graph->setUniqueColor(l);
    }
  }
  for (uint m = 0; m < grp->getNbMatrices(); ++m) {
    auto mat = grp->getMatrix(m);
    for (uint r = 0; r < mat->nbRows() - 1; ++r) {
      getGraph()->setUniqueColor(*(mat->getRow(r)));
    }
  }
}

void PB::print(std::ostream &out){
  for(auto c: constraints){
    c->print(out);
  }
}

uint PB::getSize(){
  return constraints.size();
}

uint PB::getOriginalSize(){
  return getSize(); //TODO CAN THIS DIFFER
}

void PB::setSubTheory(sptr<Group> subgroup){
  std::vector<sptr<PBConstraint> > subconsts;
  for(auto r: constraints){
    for (auto lc : r->getTerms()) {
      if (subgroup->permutes(lc.first)) {
        subconsts.push_back(r);
        break;
      }
    }
  }
  subgroup->theory=new PB(subconsts, subgroup);
}

bool PB::isSymmetry(Permutation &prm) {
  for(auto c: constraints){
    sptr<PBConstraint> symmetrical = std::make_shared<PBConstraint>(c->getWeight());
    if(!prm.getImage(c->terms,symmetrical->terms)){
      continue;
    }

    if(constraints.count(symmetrical) == 0){
      return false;
    }
  }
  return true;
}

void PB::to_solver(SLIME::SimpSolver &S) {

}
