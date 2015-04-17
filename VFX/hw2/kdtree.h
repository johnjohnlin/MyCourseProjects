#ifndef __KDTREE_H_
#define __KDTREE_H_

#include <vector>
using namespace std;

#define CHILD_NULL -1
struct Node {
	int left, right;
	int splitAxis, splitValue;
};

#define LEFT 0
#define RIGHT 1

struct DFSInfo{
	// dir is LEFT(0) or RIGHT(1)
	int nodeID, dir;
};

class KDTree {
public:
	friend bool compare_element(int a, int b);
	int KDTree_build(Node *nodes, int *coords, int n, int dim);
	void find_kNN(int *indice, int *target, int k, int noeq);
private:
	int *coords, n, dim, splitAxis, k, *target, *indice;
	long long int *sumxsq, *sumx, *dists;
	int compareDim;
	int root;
	Node* nodes;
	vector<DFSInfo> DFSInfoStack;

	int KDTree_build_rec(vector<int> &nodeIDs);
	void tree_go_down(int nodeID);
	inline void consider_node(int nodeID);
};

#endif
