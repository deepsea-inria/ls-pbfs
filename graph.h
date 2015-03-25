// Copyright (c) 2010, Tao B. Schardl
/*
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <sys/types.h>
#include <time.h>

#include "common.hpp"

#include "bag.h"

class Graph
{

 private:
  // Number of nodes
  intT nNodes;
  // Number of edges
  intT nEdges;

  intT * nodes;
  intT * edges;

  void pbfs_walk_Bag(Bag<intT>*, Bag_reducer<intT>*, unsigned int, unsigned int[]) const;
  void pbfs_walk_Pennant(Pennant<intT>*, Bag_reducer<intT>*, unsigned int,
			unsigned int[], int) const;
  void pbfs_proc_Node(const intT[], Bag_reducer<intT>*, unsigned int, unsigned int[], int) const;
  void pbfs_proc_Nodep(const intT[], int, Bag_reducer<intT>*, unsigned int, unsigned int[]) const;

  intT countEdges(Bag<intT>*);
  intT countEdgesInPennant(Pennant<intT>*, int);

  unsigned long long todval(struct timeval*);

 public:
  // Constructor/Destructor
  Graph() {}
  //  Graph(int *ir, int *jc, int m, int n, int nnz);
  ~Graph();

  // Accessors for basic graph data
  intT numNodes() const { return nNodes; }
  intT numEdges() const { return nEdges; }

  // Various BFS versions
  intT bfs(const intT s, unsigned int distances[]) const;
  intT pbfs(const intT s, unsigned int distances[]) const;

  void replace(unsigned int n, unsigned int m, intT* _nodes, intT* _edges) {
    nNodes = n;
    nEdges = m;
    nodes = _nodes;
    edges = _edges;
  }
  
};

#endif
