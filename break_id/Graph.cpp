#include "Graph.hpp"
#include "saucy.hpp"


//=====================GRAPH====================================================

//------------------------------------------------------------------------------
//           INTERACTION WITH SAUCY
void Graph::initializeGraph(uint nbNodes, uint nbEdges, std::map<uint,uint>& lit2color,  std::vector<std::vector<uint> >& neighbours){
    saucy_g = (saucy_graph*) malloc(sizeof (struct saucy_graph));
    saucy_g->colors = (int*) malloc(nbNodes * sizeof (int));
    for(uint i = 0; i < lit2color.size(); i ++){
        saucy_g->colors[i] = lit2color[i];
    }

    // now count the number of neighboring nodes
    saucy_g->adj = (int*) malloc((nbNodes + 1) * sizeof (int));
    saucy_g->adj[0] = 0;
    int ctr = 0;
    for (auto nblist : neighbours) {
        saucy_g->adj[ctr + 1] = saucy_g->adj[ctr] + nblist.size();
        ++ctr;
    }

    // finally, initialize the lists of neighboring nodes, C-style
    saucy_g->edg = (int*) malloc(saucy_g->adj[nbNodes] * sizeof (int));
    ctr = 0;
    for (auto nblist : neighbours) {
        for (auto l : nblist) {
            saucy_g->edg[ctr] = l;
            ++ctr;
        }
    }

    saucy_g->n = nbNodes;
    saucy_g->e = saucy_g->adj[nbNodes] / 2;
};

void Graph::freeGraph() {
    free(saucy_g->adj);
    free(saucy_g->edg);
    free(saucy_g->colors);
    free(saucy_g);
}


uint Graph::getNbNodesFromGraph(){
    return (uint) saucy_g->n;
}

uint Graph::getNbEdgesFromGraph() {
    return (uint) saucy_g->e;
}

uint Graph::getColorOf(uint node) {
    return saucy_g->colors[node];
}

uint Graph::nbNeighbours(uint node){
    return saucy_g->adj[node + 1] - saucy_g->adj[node];
}
uint Graph::getNeighbour(uint node, uint nbthNeighbour){
    return saucy_g->edg[saucy_g->adj[node] + nbthNeighbour];
}
void Graph::setNodeToNewColor(uint node) {
    saucy_g->colors[node] = colorcount.size() ;
}

std::vector<sptr<Permutation> > perms;

// This method is given to Saucy as a polymorphic consumer of the detected generator permutations

static int addSaucyPermutation(int n, const int *ct_perm, int nsupp, int *, void *) {
    if (n == 0 || nsupp == 0) {
        return not timeLimitPassed();
    }

    sptr<Permutation> perm = std::make_shared<Permutation>();
    for (int i = 0; i < n; ++i) {
        if (i != ct_perm[i]) {
            perm->addFromTo(i, ct_perm[i]);
        }
    }
    perms.push_back(perm);

    return not timeLimitPassed();
}

void Graph::getSymmetryGeneratorsInternal(std::vector<sptr<Permutation> >& out_perms){
    if (getNbNodes() <= 2 * nVars) { // Saucy does not like empty graphs, so don't call Saucy with an empty graph
        return;
    }

    // WARNING: make sure that maximum color does not surpass the number of colors, as this seems to give saucy trouble...
    struct saucy *s = saucy_alloc(saucy_g->n, timeLeft());
    struct saucy_stats stats;
    saucy_search(s, saucy_g, 0, addSaucyPermutation, 0, &stats);
    saucy_free(s);
    // TODO: how to check whether sg is correctly freed?

    std::swap(out_perms, perms);
}



//          END INTERACTION WITH SAUCY
//------------------------------------------------------------------------------

Graph::Graph(std::unordered_set<sptr<Clause>, UVecHash, UvecEqual>& clauses) {
  //TODO: Why are we making undirected graphs? Efficiency? In principle, a lot could be directed (e.g., only edge from clause to lit -> more in LP)

  int n = 2 * nVars + clauses.size();

    std::map<uint,uint> lit2color{};

  // Initialize colors:
  for (uint i = 0; i < 2 * nVars; ++i) {
      lit2color[i] = 0;
  }
  colorcount.push_back(2 * nVars);

  for (int i = 2 * nVars; i < n; ++i) {
      lit2color[i] = 1;
  }
  colorcount.push_back(clauses.size());

    uint nbedges = 0;

  // Initialize edge lists
  // First construct for each node the list of neighbors
  std::vector<std::vector<uint> > neighbours(n);
  // Literals have their negations as neighbors
  for (uint l = 1; l <= nVars; ++l) {
    uint posID = encode(l);
    uint negID = encode(-l);
    neighbours[posID].push_back(negID);
    neighbours[negID].push_back(posID);
      nbedges += 2;
  }
  // Clauses have as neighbors the literals occurring in them
  uint c = 2 * nVars;
  for (auto cl : clauses) {
    for (auto lit : cl->lits) {
      neighbours[lit].push_back(c);
      neighbours[c].push_back(lit);
        nbedges += 2;
    }
    ++c;
  }


  //Now, initialize the internal graph
  initializeGraph(n,nbedges,lit2color,neighbours);




  // look for unused lits, make their color unique so that no symmetries on them are found
  // useful for subgroups
  std::unordered_set<uint> usedLits;
  for (auto cl : clauses) {
    for (auto lit : cl->lits) {
      usedLits.insert(lit);
      usedLits.insert(neg(lit));
    }
  }
  for(uint i=0; i<2*nVars; ++i) {
    if(usedLits.count(i)==0) {
      setUniqueColor(i);
    }
  }

  // fix fixedLits
  setUniqueColor(fixedLits);
}
Graph::Graph(std::unordered_set<sptr<Rule>, UVecHash, UvecEqual>& rules) {
  //We create this graph:
  // color0 for positive literals
  // color1 for negative literals
  // color2 for head nodes
  // color3 for body nodes of rule-type 1 or 8 or integrity constraints
  // color4 for body nodes of rule-type 3
  // color5 for body nodes of rule type 6
  // colorn for body nodes of rule-type 2 or 5 with bound, where n is obtained from bound through the boundToColor map below.
  // arrows between positive and negative literals of the same atoms
  // arrows from head to literals occurring in it (and back)
  // arrows from body to literals occurring in it (posbodylits and negbodylits can be treated as one large list; separation is only for printing purposes) (and back)
  // arrows from head to body (and back)

  //first we collect all the bound
  auto maxcolor = 6;
  std::unordered_map<int,int> boundToColor;
  int nbextralits = 0;

  for(auto r: rules) {
    if(r->ruleType== 2 || r->ruleType == 5) {
      auto bound = r->bound;
      auto& num = boundToColor[bound];
      if(not num) {
        num = maxcolor;
        maxcolor ++;
      }
    }
    if(r->ruleType == 5 || r->ruleType == 6) {
      nbextralits += r->weights.size();
      for(auto w: r->weights) {
        auto& num = boundToColor[w];
        if(not num) {
          num = maxcolor;
          maxcolor ++;
        }
      }
    }
  }

  //Number of nodes: 2 for each variable + two for each rule (head+body)
  //The three is to make sure that all colors are used (cfr colorcount documentation)
  int n = 2 * nVars + 2 * rules.size() + 4 + nbextralits;

    std::map<uint,uint> lit2color{};
  uint current_node_index = 0;
  for (; current_node_index < 2 * nVars; ++current_node_index) {
    //Positive lits are colored with color0
      lit2color[current_node_index] = 0;
    ++current_node_index;
    //Negative lits are colored with color0
      lit2color[current_node_index] = 1;
  }
  colorcount.push_back(nVars);
  colorcount.push_back(nVars);

  //initialise the 4 extra nodes
  for(auto i: {2,3,4,5}) {
      lit2color[current_node_index] = i;
    current_node_index++;
    colorcount.push_back(1);
  }

  //Initialise colorcount!!!
  colorcount.reserve(maxcolor);
  for(auto col = 6; col < maxcolor; col ++ ) {
    colorcount.push_back(0);
  }

    uint nbedges = 0 ;
  // Initialize edge lists
  // First construct for each node the list of neighbors
  std::vector<std::vector<uint> > neighbours(n);
  // Literals have their negations as neighbors
  for (uint l = 1; l <= nVars; ++l) {
    uint posID = encode(l);
    uint negID = encode(-l);
    neighbours[posID].push_back(negID);
    neighbours[negID].push_back(posID);
      nbedges +=2;
  }

  for(auto r:rules) {
    auto bodyNode = current_node_index;
    current_node_index++;
    auto headNode = current_node_index;
    current_node_index++;

      lit2color[headNode] = 2;
    colorcount[2]++;

    if(r->ruleType == 1) {
        lit2color[bodyNode] = 3;
    }
    else if(r->ruleType == 3) {
        lit2color[bodyNode] = 4;
    }
    else if(r->ruleType == 2 || r->ruleType == 5) {
      auto color = boundToColor[r->bound];
        lit2color[bodyNode] = color;
    }
    else if(r->ruleType == 6) {
        lit2color[bodyNode] = 6;
    }

    colorcount[lit2color[bodyNode]]++;

    neighbours[bodyNode].push_back(headNode);
    neighbours[headNode].push_back(bodyNode);
      nbedges +=2;

    for(auto lit: r->headLits) {
      neighbours[lit].push_back(headNode);
      neighbours[headNode].push_back(lit);
        nbedges +=2;
    }
    if(r->ruleType != 5 && r->ruleType != 6) {
      for(auto lit: r->bodyLits) {
        neighbours[lit].push_back(bodyNode);
        neighbours[bodyNode].push_back(lit);
          nbedges +=2;
      }
    } else {
      for(uint index = 0; index < r->bodyLits.size(); index++) {
        auto lit = r->bodyLits[index];
        auto weight = r->weights[index];
        auto extranode = current_node_index;
        current_node_index++;
          lit2color[extranode] = boundToColor[weight];
        neighbours[extranode].push_back(bodyNode);
        neighbours[bodyNode].push_back(extranode);
        neighbours[extranode].push_back(lit);
        neighbours[lit].push_back(extranode);
          nbedges +=4;
      }
    }

  }

  initializeGraph(n,nbedges,lit2color, neighbours);

// look for unused lits, make their color unique so that no symmetries on them are found
// useful for subgroups
  std::unordered_set<uint> usedLits;
  for (auto cl : rules) {
    for (auto lit : cl->headLits) {
      usedLits.insert(lit);
      usedLits.insert(neg(lit));
    }
    for (auto lit : cl->bodyLits) {
      usedLits.insert(lit);
      usedLits.insert(neg(lit));
    }
  }
  for(uint i=0; i<2*nVars; ++i) {
    if(usedLits.count(i)==0) {
      setUniqueColor(i);
    }
  }

// fix fixedLits
  setUniqueColor (fixedLits);
}

Graph::Graph(std::unordered_set<sptr<PBConstraint>, UVecHash, UvecEqual>& constrs){
  //We create a graph with the following colors:
  //Literals (vars and their negation) have color 0.
  // lit+coeff combinations have a color determined by the coeff
  // constraints have a color determined by the bound (different from those from point 2).
  uint lastUsedColor=0;
  std::map<uint,uint> coeff2color;
  std::map<uint,uint> bound2color;
  auto getCoeffColor = [&coeff2color,&lastUsedColor,this] (uint coeff)->uint{
    auto& col = coeff2color[coeff];
    if(col == 0){
      lastUsedColor++;
      col=lastUsedColor;
      colorcount.resize(col+1);
    }
    return col;
  };
  auto getBoundColor = [&bound2color,this,&lastUsedColor] (uint bound)->uint{
    auto& col = bound2color[bound];
    if(col == 0){
      lastUsedColor++;
      col=lastUsedColor;
      colorcount.resize(col+1);
    }
    return col;
  };

  //The graph we are building:
  std::map<uint,uint> node2color;
  std::vector<std::vector<uint> > neighbours(2 * nVars);

  auto getNewNode = [&neighbours, &node2color]()->uint{
    auto node = neighbours.size();
    neighbours.push_back({});
    return node;
  };


  std::map<std::pair<uint,uint>, uint> litCoeff2node; //maps a lit and coefficient to a node (for node reuse and keeping graph small)
  auto getnode = [&node2color,this,&litCoeff2node,&getCoeffColor,&neighbours,&getNewNode] (uint lit, uint coeff)->uint{
    auto& node = litCoeff2node[{lit,coeff}];
    if(node == 0){
      node = getNewNode();

      auto litCol = getCoeffColor(coeff);
      node2color[node] = litCol;
      colorcount[litCol]++;

      neighbours[node].push_back(lit);
      neighbours[lit].push_back(node);

    }
    return node;
  };

  uint nbedges = 0;

  // Initialize edge lists
  // First construct for each node the list of neighbors
  // Literals have their negations as neighbors
  for (uint l = 1; l <= nVars; ++l) {
    uint posID = encode(l);
    uint negID = encode(-l);
    neighbours[posID].push_back(negID);
    neighbours[negID].push_back(posID);
    nbedges +=2;
  }


  // Initialize colors:
  for (uint i = 0; i < 2 * nVars; ++i) {
    node2color[i] = 0;
  }
  colorcount.push_back(2 * nVars);

  for(auto c: constrs){
    uint constraintNode = getNewNode();
    auto col = getBoundColor(c->getWeight());
    node2color[constraintNode] = col;
    colorcount[col]++;

    for(auto p: c->getTerms()){
      auto lit=p.first;
      auto coeff = p.second;
      auto litnode = getnode(lit, coeff);
      neighbours[litnode].push_back(constraintNode);
      neighbours[constraintNode].push_back(litnode);
      nbedges+=2;
    }
  }

  //Now, initialize the internal graph
  initializeGraph(node2color.size(),nbedges,node2color,neighbours);


  // look for unused lits, make their color unique so that no symmetries on them are found
  // useful for subgroups
  std::unordered_set<uint> usedLits;
  for (auto cl : constrs) {
    for (auto lit : cl->getTerms()) {
      usedLits.insert(lit.first);
      usedLits.insert(neg(lit.first));
    }
  }
  for(uint i=0; i<2*nVars; ++i) {
    if(usedLits.count(i)==0) {
      setUniqueColor(i);
    }
  }

  // fix fixedLits
  setUniqueColor(fixedLits);
}



Graph::~Graph() {
  freeGraph();
}

void Graph::print() {
  for (int i = 0; i < (int)getNbNodes(); ++i) {
    fprintf(stderr, "node %i with color %i has neighbours\n", i, getColorOf(i));
    for (int j = 0; j < (int)nbNeighbours(i); ++j) {
      fprintf(stderr, "%i ", getNeighbour(i,j));
    }
    fprintf(stderr, "\n");
  }
}

uint Graph::getNbNodes(){
        return getNbNodesFromGraph();
}

uint Graph::getNbEdges() {
  return getNbEdgesFromGraph();
}

void Graph::setUniqueColor(uint lit) {
    auto color = getColorOf(lit);
  uint currentcount = colorcount[getColorOf(lit)];
  if (currentcount == 1) {
    return; // color was already unique
  }
  colorcount[color] = currentcount - 1;
  setNodeToNewColor(lit);
  colorcount.push_back(1);
}

void Graph::setUniqueColor(const std::vector<uint>& lits) {
  for (auto lit : lits) {
    setUniqueColor(lit);
  }
}

void Graph::getSymmetryGenerators(std::vector<sptr<Permutation> >& out_perms) {
    out_perms.clear();
    if (timeLimitPassed()) { // do not call saucy again when interrupted by time limit previously
        return;
    }

    if (verbosity > 1) {
        std::cout << "Searching graph automorphisms with time limit: " << timeLeft() << std::endl;
    }

    getSymmetryGeneratorsInternal(out_perms);





}
