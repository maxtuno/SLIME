/* Graph is a class used as a wrapper for saucy methods
 */

#pragma once

#include "global.hpp"
#include "Algebraic.hpp"

// TODO: make a die function for mem-assignment issues?
/*static void die(string& s){
  std::cout << s << endl;
  exit(1);
}*/

class Matrix;
class Group;
struct saucy_graph;

class Graph : public std::enable_shared_from_this<Graph> {
private:

public:
  saucy_graph* saucy_g;
  std::vector<uint> colorcount; // keeps track of the number of times a color is used, so that no color is never used (seems to give Saucy trouble)
  // @INVAR: for all x: colorcount[x]>0

  Graph(std::unordered_set<sptr<Clause>, UVecHash, UvecEqual>& clauses);
  Graph(std::unordered_set<sptr<Rule>, UVecHash, UvecEqual>& rules);
  Graph(std::unordered_set<sptr<PBConstraint>, UVecHash, UvecEqual>& constrs);
  ~Graph();

private:
    //Interaction with saucy:
    void initializeGraph(uint nbNodes, uint nbEdges, std::map<uint, uint> &lit2color, std::vector<std::vector<uint> > &neighbours);
    void freeGraph();
    uint getNbNodesFromGraph();
    uint getNbEdgesFromGraph();
    uint getColorOf(uint node);
    uint nbNeighbours(uint node);
    uint getNeighbour(uint node, uint nbthNeighbour);
    void setNodeToNewColor(uint node);
    void getSymmetryGeneratorsInternal(std::vector<sptr<Permutation> > &out_perms);


public:
  uint getNbNodes();
  uint getNbEdges();
  void print();
  void setUniqueColor(uint lit);
  void setUniqueColor(const std::vector<uint>& lits);
  void getSymmetryGenerators(std::vector<sptr<Permutation> >& out_perms);
};

