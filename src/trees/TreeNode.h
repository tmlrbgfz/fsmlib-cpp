/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#ifndef FSM_TREES_TREENODE_H_
#define FSM_TREES_TREENODE_H_

#include <algorithm>
#include <memory>
#include <vector>

#include "trees/IOListContainer.h"
#include "trees/TreeEdge.h"

class TreeNode {
protected:
	/**
	The parent of this node
	*/
	TreeNode *parent;

	/**
	A list containing the children of this node
	*/
	std::vector<std::unique_ptr<TreeEdge>> children;
	mutable std::vector<std::pair<TreeNode const*,TreeEdge const*>> childIndex;

	/**
	Mark this node as deleted
	*/
	bool deleted;

	//TODO
	void add(std::vector<int>::const_iterator lstIte, const std::vector<int>::const_iterator end);
	void updateChildIndex();
    
public:
	/**
	Create a new tree node
	*/
	TreeNode();
    
	TreeNode(TreeNode const &);
	TreeNode(TreeNode &&other) = default;

    /**
     * Create a copy of this TreeNode and all its children
     */
    std::unique_ptr<TreeNode> clone() const;

	/**
	Set the node to this one parent
	@param pparent The node which will be the parent of this one
	*/
	void setParent(TreeNode *parent);

	/**
	Getter for the parent
	@return The parent
	*/
	TreeNode * getParent() const;

	/**
	Mark this node as deleted.
	If this is a leaf node, remove it from the parent's list
	of children. If the parent becomes a leaf as an effect of
	this operation and if the parent is also marked as
	deleted, remove it as well. This is continued recursively.
	*/
	void deleteNode();

    /**
    Mark this node as deleted.
    If this is a leaf node, remove it from the parent's list
    of children.
    */
    void deleteSingleNode();

	/**
	Check whether or not this node is deleted
	@return true if this node have been deleted, false otherwise
	*/
	bool isDeleted() const;

	/**
	Getter for the children
	@return The children
	*/
	std::vector<std::unique_ptr<TreeEdge>> const & getChildren() const;

	/**
	Remove a tree node from this node's children target
	@param node The target to remove
	*/
	void remove(TreeNode const *node);

	/**
	Calc the list of leaves of this tree
	@param leaves An empty list, it will be used to insert the leaves
	*/
	void calcLeaves(std::vector<TreeNode*>& leaves);

	/**
	Add and edge to this node children
	@param edge The edge to be added
	*/
	void add(std::unique_ptr<TreeEdge> &&edge);

	/**
	Check whether or not this tree node had any child or not
	@return true if it is a leaf, false otherwise
	*/
	bool isLeaf() const;

	/**
	Get the input needed to reach a tree node from this one (the node need to
	be reacheable in one step)
	@param node The node to be searched into the tree node's children target
	@return The input needed to reach the tree node
	*/
	int getIO(TreeNode const *node) const;

	/**
	Check whether or not this tree node has a specific edge or not
	@param edge The edge to be searched into the tree node's children
	@return The tree edge if found, nullptr otherwise
	*/
	TreeEdge * hasEdge(TreeEdge const *edge) const;

	/**
	Get the inputs needed to reach this specific tree node
	@return The path
	*/
	std::vector<int> getPath() const;

	/**
	 * Compare with an other three node to see if it is a super tree of this one or not
	 * @param otherNode The other tree node to compare
	 * @return true if the other tree node is a super tree of this one, false otherwise
     *
     * @note This operation really checks trees for isomorphic structure. This is not
     *       adequate when applied to output tree nodes resulting from non-observable FSMs.
	 */
	bool superTreeOf(TreeNode const *otherNode) const;

	/**
	Compare two tree node to check if they are the same or not
	@param treeNode1 The first tree node to compare
	@param treeNode2 The second tree node to compare
	@return true if they are the same, false otherwise
	*/
	friend bool operator==(TreeNode const & treeNode1, TreeNode const & treeNode2);

	/**
	Conditional addition of a new edge emanating from this node:
	If an edge labelled by x already exists, nothing is changed, otherwise a
	new edge is created, and a new leaf node is returned

	@param x FSM input
	@return	existing target node under this input,
	if no new edge has been created, or
	the new leaf node, if a new edge has been created.
	*/
	TreeNode *add(const int x);

	/**
	First delegate the work to the children, then append each input
	sequence in tcl to this node, using the special strategy of the
	add(lstIte) operation
	@param tcl The IOListContainer to be added
	*/
	void add(IOListContainer tcl);

	/**
	Append each input sequence in tcl to this node,
	using the special strategy of the add(lstIte) operation
	@param tcl The IOListContainer to be added
	*/
	void addToThisNode(IOListContainer tcl);
    void addToThisNode(const std::vector<int> &lst);
    int tentativeAddToThisNode(std::vector<int>::const_iterator start,
                               std::vector<int>::const_iterator stop) const;
    int tentativeAddToThisNode(std::vector<int>::const_iterator start,
                               std::vector<int>::const_iterator stop,
                               TreeNode const *&n) const;
	/**
	Return target TreeNode reached after following the inputs
	from a given trace in the Tree.
	@param lstIte Iterator pointing to an element of an InputTrace
	@param end Iterator pointing to the last element of an InputTrace
	@return nullptr if the input trace does not match with any path in the
	tree
	TreeNode reached after having successfully matched the whole
	input trace against the tree.
	*/
	TreeNode * after(std::vector<int>::const_iterator lstIte, const std::vector<int>::const_iterator end);
    
    void calcSize(size_t& theSize);
    
    /**
     * Perform in-order traversal and add resulting I/O-lists into
     * vector of I/O-lists.
     *
     * @param v current I/O-list, represented as vector
     * @param ioll vector of I/O-lists
     *
     */
    void traverse(std::vector<int>& v,
                  std::vector<std::vector<int>> &ioll);
                  //IOListContainer::IOListBaseType &ioll);
};
#endif //FSM_TREES_TREENODE_H_
