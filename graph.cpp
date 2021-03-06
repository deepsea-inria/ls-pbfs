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

// Graph Representation
#ifndef GRAPH_CILK
#define GRAPH_CILK

#include <assert.h>
#include <vector>
#include <sys/types.h>
#include "common.hpp"
#include <stdlib.h>

#include "graph.h"
#include "bag.h"

#define GraphDebug 0
#define RAND 0

#define NEIGHBOR_CHUNK 512
extern "C" void trivial(void);

//extern cilk::cilkview cv;

unsigned long long
Graph::todval (struct timeval *tp) {
    return tp->tv_sec * 1000 * 1000 + tp->tv_usec;
}
#if 0
Graph::Graph(int *ir, int *jc, int m, int n, int nnz)
{
  this->nNodes = m;
  this->nEdges = nnz;

  this->nodes = new int[m+1];
  this->edges = new int[nnz];

  srand(20100413);
//   this->perm = new int[m];
//   this->perm[0] = 0;
//   for (int i = 1; i < m; ++i) {
//     if (RAND) {
//       int j = rand() % (i+1);
//       this->perm[i] = this->perm[j];
//       this->perm[j] = i;
//     } else {
//       // Use the identity permutation
//       this->perm[i] = i;
//     }
//   }

//   for (int i = 0; i < m; ++i)
//     assert(perm[i] >= 0 && perm[i] < m);

  //printf("Perm done\n");

  int *w = new int[m];
  //int *v = new int[m];
  for (int i = 0; i < m; ++i) {
    w[i] = 0;
  }

  for (int i = 0; i < jc[n]; ++i)
    w[ir[i]]++;

  int prev;
  int tempnz = 0;
  for (int i = 0; i < m; ++i) {
    prev = w[i];
    w[i] = tempnz;
    tempnz += prev;
  }
  this->nodes[m] = tempnz;
  //printf("X1\n");
  //memcpy(this->nodes, w, sizeof(int) * m);
  for (int i = 0; i < m; ++i)
    this->nodes[i] = w[i];
  //printf("X2\n");
  //memcpy(v, w, m);

  for (int i = 0; i < n; ++i) {
    for (int j = jc[i]; j < jc[i+1]; j++)
      this->edges[w[ir[j]]++] = i;
  }

  if (RAND) {
    // Permute the edges
    int* tmp = new int[nnz];
    for (int i = 0; i < m; ++i) {
      int k = rand() % (this->nodes[i+1] - this->nodes[i] + 1);
      int l = 0;
      for (int j = k + this->nodes[i]; j < this->nodes[i+1]; ++j) {
	tmp[l] = this->edges[j];
	++l;
      }
      for (int j = this->nodes[i]; j < k + this->nodes[i]; ++j) {
	tmp[l] = this->edges[j];
	++l;
      }
      for (int j = this->nodes[i]; j < this->nodes[i+1]; ++j) {
	this->edges[j] = tmp[j - this->nodes[i]];
      }
//       for (int j = this->nodes[i+1]-1; j > this->nodes[i]; --j) {
// 	int k = (rand() % (j - this->nodes[i])) + this->nodes[i];
// 	int tmp = this->edges[j];
// 	this->edges[j] = this->edges[k];
// 	this->edges[k] = tmp;
//       }
    }
  }
  //printf("X3\n");

  //for (int i = 0; i < m; ++i)
  //  this->nodes[i] = this->edges + v[i];

  //this->nodes[m] = this->edges + nnz;

  delete[] w;
  //delete[] v;
}

#endif

Graph::~Graph()
{
  delete[] this->nodes;
  delete[] this->edges;
}

intT
Graph::bfs(const intT s, unsigned int distances[])
const
{
  intT *queue = new intT[nNodes];
  intT head, tail;
  intT current, newdist;

  if (s < 0 || s > nNodes)
    return -1;

  current = s;
  distances[s] = 0;
  head = 0;
  tail = 0;

  do {
    newdist = distances[current]+1;
    for (intT i = nodes[current]; i < nodes[current+1]; i++) {
      if (newdist < distances[edges[i]])
	queue[tail++] = edges[i];

      distances[edges[i]] = newdist < distances[edges[i]] ?
        newdist : distances[edges[i]];
    }
    current = queue[head++];
  } while (head <= tail);

  delete[] queue;

  return 0;
}

inline void
Graph::pbfs_walk_Bag(Bag<intT> *b,
		     Bag_reducer<intT>* next,
		     unsigned int newdist,
		     unsigned int distances[])
  const
{
  Pennant<intT> *p = NULL;

  if (b->getFill() > 0) {
    // Split the bag and recurse
    b->split(&p);
    cilk_spawn pbfs_walk_Bag(b, next, newdist, distances);
    pbfs_walk_Pennant(p, next, newdist, distances, BLK_SIZE);
  } else {
    pbfs_walk_Pennant(b->getFilling(), next, newdist, distances,
		     b->getFillingSize());
  }
//   while (b->getFill() > 0) {
//     b->split(&p);
//     cilk_spawn bfs_walk_Pennant(p, next, newdist, distances, BLK_SIZE);
//   }
//   bfs_walk_Pennant(b->getFilling(), next, newdist, distances, b->getFillingSize());
  cilk_sync;
  if (p != NULL)
    delete p;
}

inline void
Graph::pbfs_walk_Pennant(Pennant<intT> *p,
			 Bag_reducer<intT>* next,
			 unsigned int newdist,
			 unsigned int distances[],
			 int fillSize)
  const
{

  if (p->getLeft() != NULL)
    cilk_spawn pbfs_walk_Pennant(p->getLeft(), next, newdist, distances, BLK_SIZE);

  if (p->getRight() != NULL)
    cilk_spawn pbfs_walk_Pennant(p->getRight(), next, newdist, distances, BLK_SIZE);

  // Process the current element
  pbfs_proc_Node(p->getElements(), next, newdist, distances, fillSize);

  cilk_sync;

  if (p->getLeft() != NULL)
    delete p->getLeft();

  if (p->getRight() != NULL)
    delete p->getRight();
}

inline void
Graph::pbfs_proc_Node(const intT n[],
		     Bag_reducer<intT>* next,
		     unsigned int newdist,
		     unsigned int distances[],
		     int fillSize)
  const
{
  Bag<intT>* bnext = &((*next).get_reference());
  for (int j = 0; j < fillSize; j++) {
    // Scan the edges of the current node and add untouched
    // neighbors to the opposite bag
    //if ((nodes[n[j]+1] - nodes[n[j]]) < NEIGHBOR_CHUNK) {
      for (intT i = nodes[n[j]]; i < nodes[n[j]+1]; ++i) {
	//if (newdist < distances[perm[edges[i]]]) {
	if (newdist < distances[edges[i]]) {
	  (*bnext).insert(edges[i]);
	  //(*next).insert(edges[i]);
	  //printf("Test3\n");
	  //distances[perm[edges[i]]] = newdist;
	  distances[edges[i]] = newdist;
	}
	// distances[*i] = newdist < distances[*i] ? newdist : distances[*i];
      }
      //} else {
      //bfs_proc_Nodep(n, j, next, newdist, distances);
//        parfor (int i = nodes[n[j]]; i < nodes[n[j]+1]; ++i) {
// 	if (newdist < distances[edges[i]]) {
// 	  (*next)().insert(edges[i]);
// 	  //printf("Test3\n");
// 	  distances[edges[i]] = newdist;
// 	}
// 	// distances[*i] = newdist < distances[*i] ? newdist : distances[*i];
//       }
      //}
  }
//   printf("proc_Node done\n");
}

void
Graph::pbfs_proc_Nodep(const intT n[],
		      int j,
		      Bag_reducer<intT>* next,
		      unsigned int newdist,
		      unsigned int distances[])
const
{
  //  par::parallel_for(nodes[n[j]], nodes[n[j]+1], [&] (int i) {
    cilk_for (int i = nodes[n[j]]; i < nodes[n[j]+1]; ++i) {
    if (newdist < distances[edges[i]]) {
      (*next).insert(edges[i]);
      //printf("Test3\n");
      distances[edges[i]] = newdist;
    }
    // distances[*i] = newdist < distances[*i] ? newdist : distances[*i];
  }
}

intT
Graph::pbfs(const intT s, unsigned int distances[]) const
{
  //printf("Graph::pbfs running\n");

  Bag_reducer<intT> *queue[2];
  Bag_reducer<intT> b1;
  Bag_reducer<intT> b2;
  queue[0] = &b1;
  queue[1] = &b2;

  bool queuei = 1;
  unsigned int current, newdist;

  if (s < 0 || s > nNodes)
    return -1;

//   for (int i = 0; i < nNodes; ++i) {
//     distances[i] = UINT_MAX;
//   }

  //printf("Test1\n");

  //distances[perm[s]] = 0;
  distances[s] = 0;
  //printf("s = %d\tnodes = %d\n",s,nNodes);
  //bfs_proc_Node2(s, queue[queuei], 1, distances);
  // Scan the edges of the initial node and add untouched
  // neighbors to the opposite bag
  //Bag<int>* b = &((*queue[queuei]).get_reference());

  //cv.start();

  cilk_for (int i = nodes[s]; i < nodes[s+1]; ++i) {
    //  par::parallel_for(nodes[s], nodes[s+1], [&] (intT i) {
    //printf("Test\t");
    if (edges[i] != s) {
      (*queue[queuei]).insert(edges[i]);
      //distances[perm[edges[i]]] = 1;
      distances[edges[i]] = 1;
    }
    //  });
  }
  newdist = 2;

  //printf("Test2\n");
  while (!((*queue[queuei]).isEmpty())) {
    //printf("Running\n");
    (*queue[!queuei]).clear();
    //printf("done clearing\n");
    pbfs_walk_Bag(&((*queue[queuei]).get_reference()), queue[!queuei],
                  newdist, distances);
    //printf("done walking\n");
    queuei = !queuei;
    ++newdist;
  }

  //cv.stop();

  //printf("Test2\n");
  //delete queue[0];
  //delete queue[1];

  //for (int i = 0; i < nNodes; i++)
  //  printf("Distance to node %d: %d\n", i+1, distances[i]);

  return 0;
}

intT
Graph::countEdgesInPennant(Pennant<intT> *p, int fillSize)
{
  intT edgestouched = 0;
  if (p->getRight() != NULL)
    edgestouched += countEdgesInPennant(p->getRight(), BLK_SIZE);

  for (intT i = 0; i < fillSize; ++i) {
    if (nodes[(p->getElements()[i])] < nodes[(p->getElements()[i])+1])
      edgestouched += (nodes[(p->getElements()[i])+1] - nodes[(p->getElements()[i])]);
  }

  if (p->getLeft() != NULL)
    edgestouched += countEdgesInPennant(p->getLeft(), BLK_SIZE);

  return edgestouched;
}

intT
Graph::countEdges(Bag<intT> *b)
{
  Pennant<intT> *p = NULL;
  intT edgestouched = 0;
  intT pos = b->getFill()-1;

  while(pos > -1) {
    pos = b->split(&p, pos);
    edgestouched += countEdgesInPennant(p, BLK_SIZE);
  }

  edgestouched += countEdgesInPennant(b->getFilling(), b->getFillingSize());

  return edgestouched;
}

#endif
