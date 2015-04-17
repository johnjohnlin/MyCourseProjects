#include "kdtree.h"
#include <algorithm>
#include <cassert>
#include <climits>

KDTree *THIS;
bool compare_element(int a, int b)
{
	int A = (THIS->coords)[(THIS->dim) * a + (THIS->splitAxis)];
	int B = (THIS->coords)[(THIS->dim) * b + (THIS->splitAxis)];
	return A<B;
}

/** @brief build a kd-tree
 *
 *  This function builds a kd-tree recursively. It initializes some parameters
 *  then call KDTree_build_rec
 *
 *  @param[out] nodes pointer to node array for output, @c size = n @c, must be allocated beforehand
 *  @param[in] coords pointer to node coordinate, @c size = dim*n @c, int data-type for sift descriptor
 *  @param[in] n numbers of points
 *  @param[in] dim dimemsions per node
 *  @return node id for the root node
 */
int KDTree::KDTree_build(Node *nodes, int *coords, int n, int dim)
{
	// prepare for calling KdTree_build_rec
	this->nodes = nodes;
	this->coords = coords;
	this->n = n;
	this->dim = dim;
	THIS = this;
	sumxsq = new long long int [dim*2]();
	sumx   = sumxsq + dim;
	vector<int> ID_1toN(n);
	for (int i = 0; i < n; ++i) {
		ID_1toN[i] = i;
	}
	root = KDTree_build_rec(ID_1toN);
	delete[] sumxsq;
	return root;
}

/** @brief build a kd-tree, sub-routine
 *
 *  This function builds a kd-tree recursively.
 *
 *  @param[in] nodeIDs IDs of tree nodes wanted to built a sub-tree
 *  @return node id for the root node of sub-tree
 */
int KDTree::KDTree_build_rec(vector<int> &nodeIDs)
{
	int center, splitAxis, splitValue;
	size_t half = nodeIDs.size() >> 1;
	// find the axis with max variance
	for (int i = 0; i < dim; ++i) {
		sumxsq[i] = sumx[i] = 0;
	}
	for (int i = 0; i < nodeIDs.size(); ++i) {
		int *base = nodeIDs[i] * dim + coords;
		for (int j = 0; j < dim; ++j) {
			sumxsq[j] += base[j] * base[j];
			sumx[j]   += base[j];
		}
	}
	long long int maxvar = -1;
	for (int i = 0; i < dim; ++i) {
		long long int var = sumxsq[i]*nodeIDs.size() - sumx[i]*sumx[i];
		if (var > maxvar) {
			maxvar = var;
			splitAxis = i;
		}
	}
	this->splitAxis = splitAxis;
	sort(nodeIDs.begin(), nodeIDs.end(), compare_element);
	center = nodeIDs[half];
	splitValue = coords[dim*center + splitAxis];
	nodes[center].splitAxis = splitAxis;
	nodes[center].splitValue = splitValue;
#define SET_AS_LEAF(x) { nodes[x].left = nodes[x].right = CHILD_NULL; nodes[x].splitAxis = 0;}
	switch (nodeIDs.size()) {
		case 0:
		case 1:
			assert(0);
			break;
		case 2:
			nodes[center].left = nodeIDs[0];
			SET_AS_LEAF(nodeIDs[0]);
			nodes[center].right = CHILD_NULL;
			break;
		case 3:
			nodes[center].left = nodeIDs[0];
			SET_AS_LEAF(nodeIDs[0]);
			nodes[center].right = nodeIDs[2];
			SET_AS_LEAF(nodeIDs[2]);
			break;
		case 4:
		{
			vector<int> left(nodeIDs.begin(), nodeIDs.begin()+2);
			nodes[center].left = KDTree_build_rec(left);
			nodes[center].right = nodeIDs[3];
			SET_AS_LEAF(nodeIDs[3]);
			break;
		}
		default:
		{
			vector<int> left(nodeIDs.begin(), nodeIDs.begin()+half);
			nodes[center].left = KDTree_build_rec(left);
			vector<int> right(nodeIDs.begin()+(half+1), nodeIDs.end());
			nodes[center].right = KDTree_build_rec(right);
			break;
		}
	}

	return center;
}

/** @brief a sub-routine that traverse the tree until a leaf
 *
 *  Called by find_kNN
 *
 *  @param[in] nodeID ID of tree root
 */
void KDTree::tree_go_down(int nodeID)
{
	int curr = nodeID;
	while (curr != CHILD_NULL) {
		Node nodeCurr = nodes[curr];
		if (target[nodeCurr.splitAxis] <= nodeCurr.splitValue) {
			DFSInfo tmp = {curr, LEFT};
			DFSInfoStack.push_back(tmp);
			curr = nodeCurr.left;
		} else {
			DFSInfo tmp = {curr, RIGHT};
			DFSInfoStack.push_back(tmp);
			curr = nodeCurr.right;
		}
	}
}

/** @brief a sub-routine that update the shortest distance
 *
 *  Called by find_kNN
 *
 *  @param[in] nodeID ID of node to be considered
 */
inline void KDTree::consider_node(int nodeID)
{
	int *ref = coords + dim * nodeID;
	long long int sumsq = 0;
	for (int i = 0; i < dim; ++i) {
		long long int diff = ref[i] - target[i];
		sumsq += diff * diff;
	}

	for (int i = 0; i < k; ++i) {
		if (sumsq < dists[i]) {
			for (int j = k-1; j > i; --j) {
				dists[j] = dists[j-1];
				indice[j] = indice[j-1];
			}
			dists[i] = sumsq;
			indice[i] = nodeID;
			break;
		}
	}
}

/** @brief find k-NN node from a built tree
 *
 *  k > n will give out un-expected result, and maybe raise runtime error
 *
 *  @param[out] indice the indice of NN-nodes in the tree
 *  @param[in] target integer array of size dim
 *  @param[in] k the k of 'k'NN
 *  @param[in] noeq prevent finding the feature itself
 */
void KDTree::find_kNN(int *indice, int *target, int k, int noeq = -1)
{
	this->indice = indice;
	this->target = target;
	this->k = k;
	dists = new long long int[k];
	for (int i = 0; i < k; ++i) {
		dists[i] = LLONG_MAX;
	}
	DFSInfoStack.clear();
	tree_go_down(root);
	while (DFSInfoStack.size()) {
		DFSInfo tmp = DFSInfoStack.back();
		Node tmpNode = nodes[tmp.nodeID];
		DFSInfoStack.pop_back();
		if (noeq != tmp.nodeID) {
			consider_node(tmp.nodeID);
		}
		long long int axisDiff =  target[tmpNode.splitAxis] - tmpNode.splitValue;
		if (dists[k-1] >= axisDiff*axisDiff) {
			int goDownFromHere = (tmp.dir == LEFT? tmpNode.right: tmpNode.left);
			tree_go_down(goDownFromHere);
		}
	}
	delete[] dists;
}
