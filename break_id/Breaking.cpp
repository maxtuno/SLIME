#include <fstream>
#include <queue>
#include <memory>

#include "Breaking.hpp"
#include "Algebraic.hpp"
#include "Theory.hpp"
#include "global.hpp"

#include <boost/multiprecision/cpp_int.hpp>

namespace mp = boost::multiprecision;

Breaker::Breaker(sptr<Specification> origTheo) : originalTheory(origTheo) {
}


void Breaker::to_solver(SLIME::SimpSolver &S) {
    if(aspinput){
        if (verbosity > 0) {
            std::cout << "c Number of breaking clauses added: " << getAddedNbClauses() << std::endl;
            std::cout << "c Max original variable: " << nVars << std::endl;
        }
        //This number represents the falsity symbol;
        auto falsevar = nVars + nbExtraVars + 1;
        if(nbExtraVars > 0){
            std::cout << "3 " << nbExtraVars << " ";
            for(auto var = nVars + 1; var < getTotalNbVars() + 1; ++var){
                std::cout << var << " ";
            }
            std::cout << "0 0\n";
        }
        for (auto c : clauses) {
            c->printAsRule(std::cout, falsevar);
        }

        for(std::string& line: originalTheory->originalSpec) {
            std::cout << line << std::endl;
            if(line.size() > 1 && line.at(0) == 'B' && line.at(1) == '-'){
                std::cout << falsevar << std::endl;
            }
        }
        return;
    }

    if(nbPBConstraintsToGroup > -1){
        std::cout << "c Number of breaking constraints added: " << getAddedNbClauses() << std::endl;
        std::cout << "c Max original variable: " << nVars << std::endl;
        std::cout << "c Auxiliary variables: " << getAuxiliaryNbVars() << std::endl;
        if (not onlyPrintBreakers) {
            std::cout << "* #variable= " << getTotalNbVars() << " #constraint= " << getTotalNbClauses() << std::endl;
            originalTheory->print(std::cout);
        }
        for (auto c : pbConstrs) {
            c->print(std::cout);
        }
        for (auto c : clauses) {
            // clauses might be non-empty, as binary clauses are generated agnostic of PB constraints
            // also, when nbPBConstraintsToGroup==0, only clauses are generated instead of PB constraints
            sptr<PBConstraint> pbc = std::make_shared<PBConstraint>(*c);
            if(pbConstrs.count(pbc)==0){
                pbc->print(std::cout);
            }
        }

        return;
    }

    //CNF CASE:
#ifdef LOG
    if (mpi_rank == 0) {
        std::cout << "c Number of breaking clauses added: " << getAddedNbClauses() << std::endl;
        std::cout << "c Max original variable: " << nVars << std::endl;
        std::cout << "c Auxiliary variables: " << getAuxiliaryNbVars() << std::endl;
    }
#endif
    if (not onlyPrintBreakers) {
        originalTheory->to_solver(S);
    }
    for (auto c : clauses) {
        c->to_solver(S);
    }
}

void Breaker::print() {
  if(aspinput){
    if (verbosity > 0) {
      std::cout << "c Number of breaking clauses added: " << getAddedNbClauses() << std::endl;
      std::cout << "c Max original variable: " << nVars << std::endl;
    }
    //This number represents the falsity symbol;
    auto falsevar = nVars + nbExtraVars + 1;
    if(nbExtraVars > 0){
      std::cout << "3 " << nbExtraVars << " ";
      for(auto var = nVars + 1; var < getTotalNbVars() + 1; ++var){
        std::cout << var << " ";
      }
      std::cout << "0 0\n";
    }
    for (auto c : clauses) {
      c->printAsRule(std::cout, falsevar);
    }

    for(std::string& line: originalTheory->originalSpec) {
      std::cout << line << std::endl;
      if(line.size() > 1 && line.at(0) == 'B' && line.at(1) == '-'){
        std::cout << falsevar << std::endl;
      }
    }
    return;
  }

  if(nbPBConstraintsToGroup > -1){
    std::cout << "* number of breaking constraints added: " << getAddedNbClauses() << std::endl;
    std::cout << "* max original variable: " << nVars << std::endl;
    std::cout << "* auxiliary variables: " << getAuxiliaryNbVars() << std::endl;
    if (not onlyPrintBreakers) {
      std::cout << "* #variable= " << getTotalNbVars() << " #constraint= " << getTotalNbClauses() << std::endl;
      originalTheory->print(std::cout);
    }
    for (auto c : pbConstrs) {
      c->print(std::cout);
    }
    for (auto c : clauses) {
      // clauses might be non-empty, as binary clauses are generated agnostic of PB constraints
      // also, when nbPBConstraintsToGroup==0, only clauses are generated instead of PB constraints
      sptr<PBConstraint> pbc = std::make_shared<PBConstraint>(*c);
      if(pbConstrs.count(pbc)==0){
        pbc->print(std::cout);
      }
    }

    return;
  }

  //CNF CASE:
  std::cout << "c number of breaking clauses added: " << getAddedNbClauses() << std::endl;
  std::cout << "c max original variable: " << nVars << std::endl;
  std::cout << "c auxiliary variables: " << getAuxiliaryNbVars() << std::endl;
  if (not onlyPrintBreakers) {
    std::cout << "p cnf " << getTotalNbVars() << " " << getTotalNbClauses() << std::endl;
    originalTheory->print(std::cout);
  }
  for (auto c : clauses) {
    c->print(std::cout);
  }
}

void Breaker::add(sptr<Clause> cl) {
  clauses.insert(cl);
}

void Breaker::addUnary(uint l1) {
  sptr<Clause> toAdd(new Clause());
  toAdd->lits.push_back(l1);
  add(toAdd);
}

void Breaker::addBinary(uint l1, uint l2) {
  sptr<Clause> toAdd(new Clause());
  toAdd->lits.push_back(l1);
  toAdd->lits.push_back(l2);
  add(toAdd);
}

void Breaker::addTernary(uint l1, uint l2, uint l3) {
  sptr<Clause> toAdd(new Clause());
  toAdd->lits.push_back(l1);
  toAdd->lits.push_back(l2);
  toAdd->lits.push_back(l3);
  add(toAdd);
}

void Breaker::addQuaternary(uint l1, uint l2, uint l3, uint l4) {
  sptr<Clause> toAdd(new Clause());
  toAdd->lits.push_back(l1);
  toAdd->lits.push_back(l2);
  toAdd->lits.push_back(l3);
  toAdd->lits.push_back(l4);
  add(toAdd);
}

void Breaker::addBinClause(uint l1, uint l2) {
  ++nbBinClauses;
  addBinary(l1, l2);
}

void Breaker::addRegSym(sptr<Permutation> perm, std::vector<uint>& order) {
  uint current = getTotalNbClauses();
  if(useShatterTranslation){
    addShatter(perm, order, true);
  }else if(nbPBConstraintsToGroup > 0){
    addPB(perm,order,true);
  } else {
    add(perm, order, true);
  }
  nbRegClauses += getTotalNbClauses() - current;
}

void Breaker::addRowSym(sptr<Permutation> perm, std::vector<uint>& order) {
  uint current = getTotalNbClauses();
  if(useShatterTranslation){
    addShatter(perm, order, false);
  }else if(nbPBConstraintsToGroup > 0){
    addPB(perm,order,true);
  }else{
    add(perm, order, false);
  }
  nbRowClauses += getTotalNbClauses() - current;
}

std::vector<uint> getVarsToBreakOn(sptr<Permutation> perm, std::vector<uint>& order, bool limitExtraConstrs, int symBreakingFormLength){
    std::unordered_set<uint> allowedLits; // which are not the last lit in their cycle, unless they map to their negation
    for (uint i = order.size(); i > 0; --i) {
        uint lit = order.at(i - 1);
        if (allowedLits.count(lit) == 0) { // we have a last lit of a cycle
            uint sym = perm->getImage(lit);
            while (sym != lit) { // add the other lits of the cycle and the negated cycle
                allowedLits.insert(sym);
                allowedLits.insert(neg(sym));
                sym = perm->getImage(sym);
            }
        }
    }

    std::vector<uint> result;
    result.reserve(order.size());
    for (auto l: order) {
        uint sym = perm->getImage(l);
        if(l!=sym && allowedLits.count(l)>0){ result.push_back(l); }
        if(limitExtraConstrs && (int)result.size()>=symBreakingFormLength){ break; }
        if (sym == neg(l)) { break; }
    }
    return result;
}


//The number of arguments to this function is getting suspiciously high... 
void logNewBreakingClause(std::vector<uint> clauseToDecode){
  //logstream << "* I think there should now be " << nbConstraintsInProofLog << " constraints\n";

  if(verbosity > 1){
    logstream << "* new symmetry breaking clause: \n";
  }
  
  logstream << "u ";
  for(auto x: clauseToDecode){
    auto lit = decode(x);
    logstream << "1 " << (lit<0?"~":"") << "x" << abs(lit) << " ";
  }
  logstream << ">= 1 ;\n";
    //No idea why, but extra constraint seems to be used:
  ++nbConstraintsInProofLog; //new constraint
  
}


void logTseitinDef(std::vector<uint> clauseToDecode, uint tseitin){
  //logstream << "* I think there should now be " << nbConstraintsInProofLog << " constraints\n";

  logstream << "red ";
  for(auto x: clauseToDecode){
    auto lit = decode(x);
    logstream << "1 " << (lit<0?"~":"") << "x" << abs(lit) << " ";
  }
  logstream << " >= 1 ; x" << abs(decode(tseitin)) << " -> " << (decode(tseitin)>0?"1":"0") << std::endl;
  nbConstraintsInProofLog++; // derived constraint
}

void Breaker::add(sptr<Permutation> perm, std::vector<uint>& order, bool limitExtraConstrs) {
  //WARNING: VARSTOBREAKON ARE ACTUALLY LITERALS
  std::vector<uint> varsToBreakOn = getVarsToBreakOn(perm,order,limitExtraConstrs,symBreakingFormLength);

  if(logging && verbosity > 1 ){
    //This is debug output:
    logstream << "\n\n********************************************************"
      << "\n* Now we start logging the following permutation:\n*\t";
    perm->print(logstream);
    logstream << "* Note: the order in which the logging constraints are printed is not the same as order of the output."
      << "\n* In SAT, that is probably not a problem \n\n";
  }
 
  uint toDelete = 0; 

  std::map<uint, mp::cpp_int> coefmap;

  if(logging){
    // Print the initial logging constraint: the exponential encoding
    logstream << "dom ";
    uint nbVars = order.size();

    auto  coefficient = mp::cpp_int(1);
    auto  two = mp::cpp_int(2);

    for(uint i = 0; i <nbVars; i++){
      auto origLit = order[nbVars - i - 1];
      auto decodedOrigLit = decode(origLit);
      bool inverse = decodedOrigLit < 0;

      auto mappedLit = perm->getImage(origLit);
      if(mappedLit != origLit)  {
        auto decodedMappedLit = decode(mappedLit);
        auto inverseMapped = decodedMappedLit < 0;
        logstream << "-" << coefficient.str()  << " "<< (inverse?"~":"") <<  "x" << abs(decodedOrigLit) << " ";
        logstream << ""  << coefficient.str()  << " "<< (inverseMapped?"~":"") <<  "x" << abs(decodedMappedLit) << " ";
        coefmap[origLit] = coefficient;
      }

      coefficient = two * coefficient;
      
    }
    
    logstream << " >= 0" << " ; ";
    perm->printAsProjection(logstream);
    logstream << " ;  begin \n\tproofgoal #2\n\t\tp -1 -2 +\n\t\tc -1\n\tqed\n";
    nbConstraintsInProofLog = nbConstraintsInProofLog + 3; //new constraints
    if(verbosity > 3) {
      logstream << "* I think there are now " << nbConstraintsInProofLog << " constraints\n";
    }
    logstream << "qed\n\n";
    nbConstraintsInProofLog++;
    toDelete = nbConstraintsInProofLog;

  }


  uint nbVarsBroken = 0;
  uint prevLit = 0;
  uint prevSym = 0;
  uint prevTst = 0; // previous tseitin


  //Optimization actually only for the logging
  //But also simplifies code! 
  prevTst = getTseitinVar();
  addUnary(prevTst);
  if(logging){
    logTseitinDef({prevTst},prevTst);
  }


  for(auto l: order){
    if(logging && verbosity>2) {
      logstream << "* * Considering variable " << decode(l) << std::endl;
    }
    if(nbVarsBroken == varsToBreakOn.size()){
      //We found all vars to break on, so no need for the rest of the iteration
      if(logging && verbosity>2) {
        logstream << "* * All breakable variables found " << decode(l) << std::endl;
      }
      break;
    }
    uint sym = perm->getImage(l);
    if(sym == l){
      //unbroken
      if(logging && verbosity>2) {
        logstream << "* * Not part of symmetry " << decode(l) << std::endl;
      }
      continue;
    }

    //If true, this boolean means that constraints in this round are not printed because the variable was not really broken on . 
    bool OnlyLogging = (std::find( varsToBreakOn.begin(), varsToBreakOn.end(), l) == varsToBreakOn.end());

    if(OnlyLogging && not logging){
      continue;
    }

    if(OnlyLogging && verbosity>2 ){
      logstream << "* * Variable needs extra breaking clause " << decode(l) << std::endl;
    }

    uint tst = 0;
    if (nbVarsBroken == 0) {
      // adding clause for l => sym :
      // ~l | sym
      tst=prevTst;
      addBinary(neg(l), sym);
      if(logging){
        logNewBreakingClause({neg(l),sym});        
      }

    } 
   else {
      // adding clauses for (prevSym => prevLit) & prevTst => tst and tst => (l => sym)
      tst = getTseitinVar();

      // prevSym | ~prevTst | tst
      if(not OnlyLogging){ addTernary(prevSym, neg(prevTst), tst);}
      if(logging){
        logTseitinDef({prevSym, neg(prevTst), tst}, tst);
      }

      // ~prevLit | ~prevTst | tst
      if(not OnlyLogging){ addTernary(neg(prevLit), neg(prevTst), tst);}
      if(logging){
        logTseitinDef({neg(prevLit), neg(prevTst), tst}, tst);
      }

      if(useFullTranslation && not OnlyLogging){
        // adding clauses for tst => prevTst and tst => (prevSym => prevLit)
        // ~tst | prevTst
        addBinary(neg(tst),prevTst);

        // ~tst | ~prevSym | prevLit
        addTernary(neg(tst),neg(prevSym),prevLit);
      }
      if(logging){
        // Full translation is logged independently of whether the full translation is used.
        // Needed for updating the exponential encoding constraint
        logTseitinDef({neg(tst),prevTst}, neg(tst));

        logTseitinDef({neg(tst),neg(prevSym),prevLit}, neg(tst));
        logstream << "p " << toDelete << " " << nbConstraintsInProofLog << " " <<  coefmap[prevLit] << " * " << " + \n";
        nbConstraintsInProofLog++; 
        logstream << "d "<< toDelete << std::endl;
        toDelete = nbConstraintsInProofLog; 
      }

      // ~tst | ~l | sym
      if(not OnlyLogging){ addTernary(neg(tst), neg(l), sym);}
      if(logging){
        logNewBreakingClause({neg(tst), neg(l), sym});
      }
    }
    ++nbVarsBroken;

    prevLit = l;
    prevSym = sym;
    prevTst = tst;
  }

  if(logging){
    logstream << "d " << toDelete << std::endl;
  }
}

void Breaker::addShatter(sptr<Permutation> perm, std::vector<uint>& order, bool limitExtraConstrs) {
    if(logging){
      gracefulError("Shatter translation not yet implemetned");
    }

    std::vector<uint> varsToBreakOn = getVarsToBreakOn(perm,order,limitExtraConstrs,symBreakingFormLength);

    int nrExtraConstrs = 0;
    uint prevLit = 0;
    uint prevSym = 0;
    uint prevTst = 0; // previous tseitin
    for (auto l: varsToBreakOn) {
        uint sym = perm->getImage(l);
        uint tst = 0;
        if (nrExtraConstrs == 0) {
            // adding clause for l => sym :
            // ~l | sym
            addBinary(neg(l), sym);
        } else if (nrExtraConstrs == 1) {
            tst = getTseitinVar();
            // clause(-z, -x, p[x], 0);
            addTernary(neg(prevLit), neg(l), sym);
            // clause(-z, vars+1, 0);
            addBinary(neg(prevLit), tst);
            // clause(p[z], -x, p[x], 0);
            addTernary(prevSym, neg(l), sym);
            // clause(p[z], vars+1, 0);
            addBinary(prevSym, tst);
        } else {
            tst = getTseitinVar();
            // clause(-vars, -z, -x, p[x], 0);
            addQuaternary(neg(prevTst), neg(prevLit), neg(l), sym);
            // clause(-vars, -z, vars+1, 0);
            addTernary(neg(prevTst), neg(prevLit), tst);
            // clause(-vars, p[z], -x, p[x], 0);
            addQuaternary(neg(prevTst), prevSym, neg(l), sym);
            // clause(-vars, p[z], vars+1, 0);
            addTernary(neg(prevTst), prevSym, tst);
        }
        ++nrExtraConstrs;

        prevLit = l;
        prevSym = sym;
        prevTst = tst;
    }
}

/*
Assume some literal-mapping symmetry s, and 3 ordered variables x<y<z. Bart's initial PB sbf is:

-4 x 4 s(x) -2 y 2 s(y) -1 z 1 s(z) >= 0

(though it's partial as there might be variables beyond x,y,z).

We can split this constraint by introducing auxiliary variables o, p, q, r. Then we get

1 o >= 1
-2 o -1 x 1 s(x) 1 p >= -1
-2 p -1 y 1 s(y) 1 q >= -1
-2 q -1 z 1 s(z) 1 r >= -1
-1 r >= -1 (trivial)

Eliminating o, p, q, r by canceling addition implies the above sbf. And eliminating any single auxiliary variable multiplies some set of coefficients by 2.

Note that the core constraint

-2 o -1 x 1 s(x) 1 p >= -1

is logically equivalent to the sbf encoding with 3 3-clauses used in the CNF context.
 */

sptr<PBConstraint> getPBConstraint(const std::vector<uint>& lits, uint firstTseitin, uint lastTseitin){
  int weight = 0;
  uint coeff = 1 << (lits.size()/2);
  if(firstTseitin!=0){ weight -= coeff; }
  if(lastTseitin!=0){ weight += 1; }
  sptr<PBConstraint> pbc = std::make_shared<PBConstraint>(weight);
  if(firstTseitin!=0){ pbc->addTerm(firstTseitin,-coeff); }
  if(lastTseitin!=0){ pbc->addTerm(lastTseitin,1); }
  for(uint i=0; i<lits.size(); i+=2){
    coeff = coeff >> 1; // divide coeff by 2
    pbc->addTerm(lits[i],-coeff);
    pbc->addTerm(lits[i+1],coeff);
  }
  return pbc;
}

void Breaker::addPB(sptr<Permutation> perm, std::vector<uint>& order, bool limitExtraConstrs) {
  if(logging){
      gracefulError("PB Logging not yet implemetned");
  }
  std::vector<uint> varsToBreakOn = getVarsToBreakOn(perm,order,limitExtraConstrs,symBreakingFormLength);

  // Create sets of lits that make up each constraint
  std::vector<std::vector<uint> > constrLits; // lits for each pb constraint
  for(auto l: varsToBreakOn){
      if(constrLits.size()==0 || (int)constrLits.back().size()>=2*nbPBConstraintsToGroup){
          constrLits.push_back({});
      }
      constrLits.back().push_back(l);
      constrLits.back().push_back(perm->getImage(l));
  }

  // For each set of lits, create the corresponding PB constraint
  // We optimize the number of tseitin variables
  if(constrLits.size()==0){
    return;
  }
  if(constrLits.size()==1){
    pbConstrs.insert(getPBConstraint(constrLits.front(),0,0));
    return;
  }

  // else, we have at least 2 constraints, of which the first and last form a special tseitin case
  uint firstTseitin = 0;
  uint lastTseitin = getTseitinVar();
  pbConstrs.insert(getPBConstraint(constrLits.front(),firstTseitin,lastTseitin));

  for(int i=1; i<(int)constrLits.size()-1; ++i){
    firstTseitin=lastTseitin;
    lastTseitin=getTseitinVar();
    pbConstrs.insert(getPBConstraint(constrLits[i],firstTseitin,lastTseitin));
  }

  firstTseitin = lastTseitin;
  lastTseitin = 0;
  pbConstrs.insert(getPBConstraint(constrLits.back(),firstTseitin,lastTseitin));
}

uint Breaker::getAuxiliaryNbVars() {
  return nbExtraVars;
}

uint Breaker::getTotalNbVars() {
  return nVars + nbExtraVars;
}

uint Breaker::getAddedNbClauses() {
  return clauses.size() + pbConstrs.size();
}

uint Breaker::getTotalNbClauses() {
  return originalTheory->getSize() + getAddedNbClauses();
}

uint Breaker::getNbBinClauses() {
  return nbBinClauses;
}

uint Breaker::getNbRowClauses() {
  return nbRowClauses;
}

uint Breaker::getNbRegClauses() {
  return nbRegClauses;
}

uint Breaker::getTseitinVar() {
  ++nbExtraVars;
  auto tst = encode(getTotalNbVars());
  if(logging && verbosity > 1){
    logstream << "* We introduce a new Tseitin variable x" << decode(tst) << std::endl
            << "* The following map redundancy rules contain its definition.\n\n";
  }
  return tst;
}
