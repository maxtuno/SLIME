BreakID 2.5
===========
Welcome to the repository of the second version of BreakID, a new symmetry detecting and breaking preprocessor for SAT solvers. A summary of BreakID's symmetry breaking techniques can be found in the file "BreakIDGlucose2.pdf", the solver description provided to the SATRACE '15 competition. The submodule "paper" contains a more detailed description of BreakID's symmetry breaking techniques (to obtain it, run `git submodule init; git submodule update --recursive --init`). The folder "experiments" contains results of experiments run in February 2016. The folder "test_cnfs" contains highly symmetrical test cnfs. Particular attention goes to instances in `channel` and `counting`, which both exhibit row interchangeability due to high-level variable or value interchangeability.

-----------
Download
-----------
A statically compiled 64 bit Unix binary is provided in the "Downloads" section.

-----------
Compile
-----------
Simply run "make" in the "src/" folder. The executable "BreakID" will be compiled into that folder. To clean the build, run "make clean" in "src/".

-----------
Run
-----------
Run
```
./BreakID -h
```
to get information on how to use BreakID and what its options are.

For example
```
./BreakID -f ../test_cnfs/pigeonhole/hole002.cnf -v 0 -t 300 -s 100
```
uses the same arguments as in the SATRACE '15 competition.

As far as output is concerned, all verbosity output is sent to stderr. stdout will contain a Dimacs CNF file, with as first three lines
```
c number of breaking clauses added: <total number of symmetry breaking clauses added to the cnf>
c max original variable: <highest variable in input cnf>
p cnf <new number of variables> <new number of clauses>
```
followed by a set of clauses equivalent to the original CNF theory augmented with symmetry breaking clauses.

-----------
ASP support
-----------

Starting from version 2.2, BreakID supports ASP files in the intermediate lparse/smodels or gringo/clasp format. 
A typical workflow for ASP solving using BreakID is:

```
gringo ../test_asp/simple/swaplits.asp -o smodels | ./BreakID -asp
```

where `-v 0 -t 300 -s 100` is the configuration used in various competitions. 

-----------
PB support
-----------

Starting from version 2.5, BreakID supports pseudoboolean (PB) files in the .opb format. 
A typical workflow for PB solving using BreakID is:

```
cat ../test_opb/hole-3-2.opb | ./BreakID -pb 1
```

-----------
Changelog
-----------
1.0: see [bitbucket.org/krr/symbreaker](https://bitbucket.org/krr/symbreaker)    
2.0: rewrite of BreakID    
2.1: streamlined command-line usage and lowered memory overhead    
2.2: Added support for ASP files    
2.3: Fixed bug and added extra command line options    
2.4: Input instance can be passed both via input stream as well as input file    
2.5: Added support for PB files (.opb)    

-----------
Authors
-----------
Jo Devriendt  
Bart Bogaerts  

-----------
Cite as
-----------
```text
@InProceedings{breakid2016,
author="Devriendt, Jo
and Bogaerts, Bart
and Bruynooghe, Maurice
and Denecker, Marc",
editor="Creignou, Nadia
and Le Berre, Daniel",
title="Improved Static Symmetry Breaking for {SAT}",
booktitle="Theory and Applications of Satisfiability Testing -- SAT 2016",
year="2016",
publisher="Springer International Publishing",
address="Cham",
pages="104--122",
abstract="An effective SAT preprocessing technique is the construction of symmetry breaking formulas: auxiliary clauses that guide a SAT solver away from needless exploration of symmetric subproblems. However, during the past decade, state-of-the-art SAT solvers rarely incorporated symmetry breaking. This suggests that the reduction of the search space does not outweigh the overhead incurred by detecting symmetry and constructing symmetry breaking formulas. We investigate three methods to construct more effective symmetry breaking formulas. The first method simply improves the encoding of symmetry breaking formulas. The second detects special symmetry subgroups, for which complete symmetry breaking formulas exist. The third infers binary symmetry breaking clauses for a symmetry group as a whole rather than longer clauses for individual symmetries. We implement these methods in a symmetry breaking preprocessor, and verify their effectiveness on both hand-picked problems as well as the 2014 SAT competition benchmark set. Our experiments indicate that our symmetry breaking preprocessor improves the current state-of-the-art in static symmetry breaking for SAT and has a sufficiently low overhead to improve the performance of modern SAT solvers on hard combinatorial instances.",
isbn="978-3-319-40970-2",
doi="10.1007/978-3-319-40970-2\_8",
url="https://bitbucket.org/krr/breakid"
}
```
