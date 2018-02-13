/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#include "trees/Tree.h"

using namespace std;

void Tree::calcLeaves()
{
	leaves.clear();
	root->calcLeaves(leaves);
}

void Tree::remove(const shared_ptr<TreeNode> thisNode, const shared_ptr<TreeNode> otherNode)
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

void Tree::printChildren(ostream & out, const shared_ptr<TreeNode> top, const shared_ptr<int> idNode) const
{
	int idNodeBase = *idNode;
	for (auto const &edge : top->getChildren())
	{
		out << idNodeBase << " -> " << ++ *idNode << "[label = \"" << edge->getIO() << "\" ];" << endl;
		printChildren(out, edge->getTarget(), idNode);
    }
}

bool Tree::inPrefixRelation(vector<int> aPath, vector<int> bPath)
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

Tree::Tree(const shared_ptr<TreeNode> root, const shared_ptr<FsmPresentationLayer> presentationLayer)
	: root(root), presentationLayer(presentationLayer)
{

}

vector<TreeNode*> Tree::getLeaves()
{
	calcLeaves();
	return leaves;
}

shared_ptr<TreeNode> Tree::getRoot() const
{
	return root;
}

shared_ptr<Tree> Tree::getSubTree(const shared_ptr<InputTrace> alpha)
{
    TreeNode *afterAlpha = root->after(alpha->cbegin(), alpha->cend());
    shared_ptr<TreeNode> cpyNode { afterAlpha->clone().release() };
    return make_shared<Tree>(cpyNode, presentationLayer);
}

shared_ptr<TreeNode> Tree::getSubTree(shared_ptr< vector<int> > alpha) {
    
    return std::shared_ptr<TreeNode>(root->after(alpha->begin(),alpha->end()));
}

IOListContainer Tree::getIOLists()
{
	shared_ptr<vector<vector<int>>> ioll = make_shared<vector<vector<int>>>();
	calcLeaves();

	for (TreeNode *n : leaves)
	{
		ioll->push_back(n->getPath());
	}

	return IOListContainer(ioll, presentationLayer);
}



IOListContainer Tree::getIOListsWithPrefixes()
{
    shared_ptr<vector<vector<int>>> ioll = make_shared<vector<vector<int>>>();
    
    // Create empty I/O-list as vector
    vector<int> thisVec; 
    
    // Perform in-order traversal of the tree
    // and create all I/O-lists.
    root->traverse(thisVec,*ioll);
    
    return IOListContainer(ioll, presentationLayer);
}

void Tree::remove(const shared_ptr<Tree> otherTree)
{
	remove(getRoot(), otherTree->getRoot());
}

void Tree::toDot(ostream & out)
{
	out << "digraph Tree {" << endl;
	out << "\trankdir=TB;" << endl;//Top -> Bottom, to create a vertical graph
	out << "\tnode [shape = circle];" << endl;
	shared_ptr<int> id = make_shared<int>(0);
	printChildren(out, root, id);
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

void Tree::addToRoot(const vector<int> &lst)
{
    root->addToThisNode(lst);
}

void Tree::unionTree(const shared_ptr<Tree> otherTree)
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


size_t Tree::size() {
    
    size_t theSize = 0;
    root->calcSize(theSize);
    return theSize;
}

shared_ptr<Tree> Tree::getPrefixRelationTree(const shared_ptr<Tree> & b)
{
    IOListContainer aIOlst = getIOLists();
    IOListContainer bIOlst = b->getIOLists();

    shared_ptr<vector<vector<int>>> aPrefixes = aIOlst.getIOLists();
    shared_ptr<vector<vector<int>>> bPrefixes = bIOlst.getIOLists();

    shared_ptr<TreeNode> r = make_shared<TreeNode>();
    shared_ptr<Tree> tree = make_shared<Tree>(r, presentationLayer);

    if (aPrefixes->at(0).size() == 0 && bPrefixes->at(0).size() == 0)
    {
        return tree;
    }

    if (aPrefixes->at(0).size() == 0)
    {
        return b;
    }
    if (bPrefixes->at(0).size() == 0)
    {
        return shared_from_this();
    }


    for (auto aPrefix : *aPrefixes)
    {
        for (auto bPrefix : *bPrefixes)
        {
            if (inPrefixRelation(aPrefix, bPrefix))
            {
                r->addToThisNode(aPrefix);
                r->addToThisNode(bPrefix);
            }
        }
    }
    return tree;

}



int Tree::tentativeAddToRoot(const std::vector<int>& alpha) {
    return root->tentativeAddToThisNode(alpha.cbegin(), alpha.cend());
}





int Tree::tentativeAddToRoot(SegmentedTrace& alpha) const {
    
    int r;
    TreeNode const *n = root.get();
    
    for ( size_t i = 0; i < alpha.size(); i++ ) {
        shared_ptr<TraceSegment> seg = alpha.getSegments().at(i);
        r = n->tentativeAddToThisNode(seg->get()->cbegin(), seg->get()->cend(), n);
        if ( r > 0 ) return r;
    }
    
    return 0;

}










