/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#include "trees/Tree.h"

void Tree::calcLeaves()
{
	leaves.clear();
	root->calcLeaves(leaves);
}

void Tree::remove(TreeNode *thisNode, TreeNode const *otherNode)
{
	thisNode->deleteNode();

	for (auto const &e : thisNode->getChildren())
	{
		TreeEdge *eOther = otherNode->hasEdge(e.get());
		if (eOther != nullptr)
		{
			remove(e->getTarget(), eOther->getTarget());
		}
	}
}

void Tree::printChildren(std::ostream & out, const TreeNode *top, int &idNode) const
{
	int idNodeBase = idNode;
	for (auto const &edge : top->getChildren())
	{
		out << idNodeBase << " -> " << ++ idNode << "[label = \"" << edge->getIO() << "\" ];" << std::endl;
		printChildren(out, edge->getTarget(), idNode);
    }
}

bool Tree::inPrefixRelation(std::vector<int> aPath, std::vector<int> bPath) const
{
    if (aPath.size() == 0 || bPath.size() == 0)
        return false;
    for (unsigned i = 0; i<aPath.size() && i < bPath.size(); i++)
    {
        if (aPath[i] != bPath[i])
        {
            return false;
        }
    }
    return true;
}

Tree::Tree(std::unique_ptr<TreeNode> &&root, std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
	: root(std::move(root)), presentationLayer(std::move(presentationLayer))
{

}

Tree::Tree(std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
	: root(new TreeNode()), presentationLayer(std::move(presentationLayer))
{

}

Tree::Tree(Tree const &other)
    : root(other.root->clone()), presentationLayer(other.presentationLayer->clone()) {
}

std::unique_ptr<Tree> Tree::clone() const {
    return std::unique_ptr<Tree>(new Tree(*this));
}

std::vector<TreeNode*> Tree::getLeaves()
{
	calcLeaves();
	return leaves;
}

TreeNode* Tree::getRoot() const
{
	return root.get();
}

std::unique_ptr<Tree> Tree::getSubTree(const InputTrace *alpha) const {
    TreeNode *afterAlpha = root->after(alpha->cbegin(), alpha->cend());
    std::unique_ptr<TreeNode> cpyNode { afterAlpha->clone() };
    return std::unique_ptr<Tree> { new Tree(std::move(cpyNode), std::move(presentationLayer->clone())) };
}

TreeNode* Tree::getSubTree( std::vector<int> const *alpha) const {
    
    return root->after(alpha->begin(),alpha->end());
}

IOListContainer Tree::getIOLists()
{
    std::shared_ptr<std::vector<std::vector<int>>> ioll = std::make_shared<std::vector<std::vector<int>>>();
	calcLeaves();

	for (TreeNode *n : leaves)
	{
		ioll->push_back(n->getPath());
	}

	return IOListContainer(ioll, std::shared_ptr<FsmPresentationLayer>(presentationLayer->clone().release()));
}



IOListContainer Tree::getIOListsWithPrefixes() const
{
    std::shared_ptr<std::vector<std::vector<int>>> ioll = std::make_shared<std::vector<std::vector<int>>>();
    
    // Create empty I/O-list as vector
    std::vector<int> thisVec; 
    
    // Perform in-order traversal of the tree
    // and create all I/O-lists.
    root->traverse(thisVec, *ioll);
    
    return IOListContainer(ioll, std::shared_ptr<FsmPresentationLayer>(presentationLayer->clone().release()));
}

void Tree::remove(Tree const *otherTree)
{
	remove(getRoot(), otherTree->getRoot());
}

void Tree::toDot(std::ostream & out) const
{
	out << "digraph Tree {" << std::endl;
	out << "\trankdir=TB;" << std::endl;//Top -> Bottom, to create a vertical graph
	out << "\tnode [shape = circle];" << std::endl;
	int id = 0;
	printChildren(out, root.get(), id);
	out << "}";
}

IOListContainer Tree::getTestCases()
{
	return getIOLists();
}

void Tree::add(const IOListContainer & tcl)
{
	root->add(tcl);
}

void Tree::addToRoot(const IOListContainer & tcl)
{
	root->addToThisNode(tcl);
}

void Tree::addToRoot(const std::vector<int> &lst)
{
    root->addToThisNode(lst);
}

void Tree::unionTree(Tree *otherTree)
{
	addToRoot(otherTree->getIOLists());
}

void Tree::addAfter(const InputTrace & tr, const IOListContainer & cnt)
{
	TreeNode *n = root->after(tr.cbegin(), tr.cend());

	if (n == nullptr)
	{
		return;
	}
	n->addToThisNode(cnt);
}


size_t Tree::size() const {
    size_t theSize = 0;
    root->calcSize(theSize);
    return theSize;
}

std::unique_ptr<Tree> Tree::getPrefixRelationTree(Tree *b) {
    IOListContainer aIOlst = getIOLists();
    IOListContainer bIOlst = b->getIOLists();

    auto aPrefixes = aIOlst.getIOLists();
    auto bPrefixes = bIOlst.getIOLists();

    std::unique_ptr<Tree> tree { new Tree(presentationLayer->clone()) };

    if (aPrefixes->at(0).size() == 0 && bPrefixes->at(0).size() == 0) {
        return tree;
    }

    if (aPrefixes->at(0).size() == 0) {
        return b->clone();
    }
    if (bPrefixes->at(0).size() == 0) {
        return this->clone();
    }


    TreeNode *root = tree->getRoot();
    for (auto aPrefix : *aPrefixes) {
        for (auto bPrefix : *bPrefixes) {
            if (inPrefixRelation(aPrefix, bPrefix)) {
                root->addToThisNode(aPrefix);
                root->addToThisNode(bPrefix);
            }
        }
    }
    return tree;
}

int Tree::tentativeAddToRoot(const std::vector<int>& alpha) const {
    return root->tentativeAddToThisNode(alpha.cbegin(), alpha.cend());
}

int Tree::tentativeAddToRoot(SegmentedTrace& alpha) const {
    int r;
    TreeNode const *n = root.get();
    
    for ( size_t i = 0; i < alpha.size(); i++ ) {
        std::shared_ptr<TraceSegment> seg = alpha.getSegments().at(i);
        r = n->tentativeAddToThisNode(seg->get()->cbegin(), seg->get()->cend(), n);
        if ( r > 0 ) return r;
    }
    
    return 0;
}
