#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include <iostream>
#include <limits.h>
#include <sys/mman.h>

#include "common.hpp"

#include "util.h"

using namespace std;

//cilk::cilkview cv;

const bool DEBUG = false;



#include "graphfileshared.hpp"

intT nb_visited;


/*---------------------------------------------------------------------*/

namespace pasl {
namespace graph {
  
bool should_disable_random_permutation_of_vertices;

template <class Adjlist, class LSGraph>
void convert(const Adjlist& adj, LSGraph& lsg) {
  intT n = adj.get_nb_vertices();
  intT m = adj.nb_edges;
  intT* nodes = newA(intT, n+1);
  intT* edges = newA(intT, m);
  intT nptr = 0;
  for (intT i = 0; i < n; i++) {
    intT deg = adj.adjlists[i].get_out_degree();
    nodes[i] = nptr;
    nptr += deg;
  }
  nodes[n] = nptr;
  for (intT i = 0; i < n; i++) {
    intT deg = adj.adjlists[i].get_out_degree();
    intT* nghbrs = adj.adjlists[i].get_out_neighbors();
    intT* dst = &edges[nodes[i]];
    for (intT j = 0; j < deg; j++) {
      dst[j] = nghbrs[j];
    }
  }
  lsg.replace(n, m, nodes, edges);
}

 
}
}


/*---------------------------------------------------------------------*/



int main(int argc, char** argv) {

  using vtxid_type = intT;
  using adjlist_seq_type = pasl::graph::flat_adjlist_seq<vtxid_type>;
  using adjlist_type = pasl::graph::adjlist<adjlist_seq_type>;
  
  using vtxid_type = typename adjlist_type::vtxid_type;
  Graph lsg;
  intT source;
  unsigned int* distances;
  auto init = [&] {
    pasl::graph::should_disable_random_permutation_of_vertices = pasl::util::cmdline::parse_or_default_bool("should_disable_random_permutation_of_vertices", false, false);
    adjlist_type graph;
    source = (intT) pasl::util::cmdline::parse_or_default_long("source", 0);
    pasl::util::cmdline::argmap_dispatch tmg;
    tmg.add("from_file",          [&] { load_graph_from_file(graph); });
    tmg.add("by_generator",       [&] { generate_graph(graph); });
    pasl::util::cmdline::dispatch_by_argmap(tmg, "load");
    convert(graph, lsg);
    print_adjlist_summary(graph);
    distances = newA(unsigned int, graph.get_nb_vertices());
    mlockall(0);
  };
  auto run = [&] (bool sequential) {
    par::parallel_for((intT)0, (intT)lsg.numNodes(), [&] (intT i) {
      distances[i] = UINT_MAX;
    });
    lsg.pbfs(source, distances);
  };
  auto output = [&] {
    nb_visited = 0;
    for (intT i = 0; i < lsg.numNodes(); i++) {
      if (distances[i] != UINT_MAX) {
        nb_visited++;
      }
    }
    std::cout << "nb_visited\t" << nb_visited << std::endl;
  };
  auto destroy = [&] {
    free(distances);
  };
  pasl::sched::launch(argc, argv, init, run, output, destroy);
  
  return 0;
}

/*

// Helper function for checking correctness of result
static bool
check(unsigned int distances[],
      unsigned int distverf[],
      int nodes)
{
  for (int i = 0; i < nodes; i++) {
    if (distances[i] != distverf[i]) {
      printf("distances[%d] = %d; distverf[%d] = %d\n", i, distances[i], i, distverf[i]);
      return false;
    }
  }

  return true;
}

int
main2 (int argc, char** argv)
{
  Graph *graph;
  int s;
  unsigned long long runtime_ms;

  BFSArgs bfsArgs = parse_args(argc, argv);

  if (DEBUG)
    printf("algorithm = %s\n", ALG_NAMES[bfsArgs.alg_select]);

  if (parseBinaryFile(bfsArgs.filename, &graph) != 0)
    return -1;

  // Initialize extra data structures
  int numNodes = graph->numNodes();

  unsigned int *distances = new unsigned int[numNodes];

  // Pick a starting node
  s = 0;

  // Format test name for cilkview printing
  char *testname;
  size_t found2 = bfsArgs.filename.find_last_of(".");
  size_t found1 = bfsArgs.filename.find_last_of("/\\");
  testname = new char [found2 - found1 + 32];
  std::strcpy(testname, (bfsArgs.filename.substr(found1+1, found2-found1-1)).c_str());

  // Execute BFS
  parfor (int i = 0; i < numNodes; ++i) {
    distances[i] = UINT_MAX;
  }


  switch (bfsArgs.alg_select) {
  case BFS:
    std::strncat(testname, "-BFS", 4);
    //cv.start();
    graph->bfs(s, distances);
    //cv.stop();
    //runtime_ms = cv.accumulated_milliseconds();
    //cv.dump(testname);
    break;
  case PBFS:
    std::strncat(testname, "-PBFS", 5);
    //cv.start();
    graph->pbfs(s, distances);
    //cv.stop();
    //runtime_ms = cv.accumulated_milliseconds();
    //cv.dump(testname);
    break;
  default: break;
  }

  // Verify correctness
  if (bfsArgs.check_correctness) {

    unsigned int *distverf = new unsigned int[numNodes];
    parfor (int i = 0; i < numNodes; ++i) {
      distverf[i] = UINT_MAX;
    }

    graph->bfs(s, distverf);
    if (!check(distances, distverf, numNodes))
      fprintf(stderr, "Error found in %s result.\n", ALG_NAMES[bfsArgs.alg_select]);
  }

  // Print results if debugging
  if (DEBUG) {
    for (int i = 0; i < numNodes; i++)
      printf("Distance to node %d: %d\n", i+1, distances[i]);
  }

  // Print runtime result
  switch(bfsArgs.alg_select) {
  case BFS:
    printf("BFS on %s: %f seconds\n", bfsArgs.filename.c_str(), runtime_ms/1000.0);
    break;
  case PBFS:
    printf("PBFS on %s: %f seconds\n", bfsArgs.filename.c_str(), runtime_ms/1000.0);
    break;
  default: break;
  }

  return 0;
}
 */
