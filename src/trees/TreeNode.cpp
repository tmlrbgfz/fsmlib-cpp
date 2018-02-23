/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 *
 * Licensed under the EUPL V.1.1
 */
#include "trees/TreeNode.h"
#include "utils/prepostconditions.h"
#include <deque>

using namespace std;

TreeNode::TreeNode()
: parent(nullptr), deleted(false) {
}

TreeNode::TreeNode(TreeNode const &other) {
    children.reserve(other.children.size());

    for(auto const &child : other.children) {
        children.emplace_back(child->clone());
        children.back()->getTarget()->setParent(this);
    }
    updateChildIndex();
    parent = nullptr;
}

std::unique_ptr<TreeNode> TreeNode::clone() const {
    return std::unique_ptr<TreeNode>(new TreeNode(*this));
}

void TreeNode::updateChildIndex() {
    childIndex.clear();
    for(auto const &child : children) {
        childIndex.emplace_back(std::pair<TreeNode const*, TreeEdge const*>(child->getTarget(), child.get()));
    }
}

void TreeNode::setParent(TreeNode *pparent) {
    parent = pparent;
}

TreeNode * TreeNode::getParent() const {
    return parent;
}

void TreeNode::deleteSingleNode() {
    deleted = true;
    
    children.clear();

    TreeNode const *c = this;
    TreeNode *t = parent;
    
    if (t != nullptr) {
        t->remove(c);
    }
}

void TreeNode::deleteNode() {
    deleted = true;
    
    children.clear();
    
    TreeNode const *c = this;
    TreeNode *t = parent;
    
    while (t != nullptr) {
        t->remove(c);
        if(t->getChildren().empty()) {
            t->deleteNode();
        } else {
            break;
        }
        c = t;
        t = t->getParent();
    }
}

bool TreeNode::isDeleted() const
{
    return deleted;
}

std::vector<std::unique_ptr<TreeEdge>> const & TreeNode::getChildren() const
{
    return children;
}

void TreeNode::remove(TreeNode const *node) {
    auto edgeToRemove = std::find_if(children.begin(), children.end(), [node](std::unique_ptr<TreeEdge> const &edge){
        return edge->getTarget() == node;
    });
    children.erase(edgeToRemove);
    auto indexIter = std::find_if(childIndex.begin(), childIndex.end(), [node](std::pair<TreeNode const*, TreeEdge const*> const &value){
        return value.first == node;
    });
    childIndex.erase(indexIter);
}

void TreeNode::calcLeaves(std::vector<TreeNode*> &leaves) {
    if (isLeaf()) {
        leaves.push_back(this);
    } else {
        for (auto &child : children) {
            child->getTarget()->calcLeaves(leaves);
        }
    }
}

void TreeNode::add(std::unique_ptr<TreeEdge> &&edge) {
    edge->getTarget()->setParent(this);
    childIndex.emplace_back(std::make_pair(edge->getTarget(), edge.get()));
    children.emplace_back(std::move(edge));
}

bool TreeNode::isLeaf() const {
    return children.empty();
}

int TreeNode::getIO(TreeNode const *node) const {
    auto edge = std::find_if(childIndex.begin(), childIndex.end(), 
        [&node](std::pair<TreeNode const*, TreeEdge const*> const &child){
            return child.first == node;
    });
    Expects(edge != childIndex.end());
    return (*edge).second->getIO();
}

TreeEdge * TreeNode::hasEdge(TreeEdge const *edge) const
{
    auto edgeIter = std::find_if(children.begin(), children.end(), [edge](std::unique_ptr<TreeEdge> const &child){
        return child->getIO() == edge->getIO();
    });
    if(edgeIter != children.end()) {
        return edgeIter->get();
    } else {
        return nullptr;
    }
}

vector<int> TreeNode::getPath() const
{
    std::deque<int> path;
    
    TreeNode const *m = this;
    TreeNode const *n = parent;
    
    while (n != nullptr)
    {
        path.insert(path.begin(), n->getIO(m));
        m = n;
        n = n->getParent();
    }
    std::vector<int> result(path.begin(), path.end());
    
    return result;
}

bool TreeNode::superTreeOf(TreeNode const *otherNode) const
{
    
    if (children.size() < otherNode->children.size())
    {
        return false;
    }
    
    for (auto &eOther : otherNode->children)
    {
        int y = eOther->getIO();
        bool yFound = false;
        
        for (auto &eMine : children)
        {
            if (y == eMine->getIO())
            {
                if (!eMine->getTarget()->superTreeOf(eOther->getTarget()))
                {
                    return false;
                }
                yFound = true;
                break;
            }
        }
        
        /*If this node does not have an outgoing edge labelled with y, the nodes differ.*/
        if (!yFound)
        {
            return false;
        }
    }
    return true;
}

bool operator==(TreeNode const & treeNode1, TreeNode const & treeNode2)
{
    if (treeNode1.children.size() != treeNode2.children.size())
    {
        return false;
    }
    
    if (treeNode1.deleted != treeNode2.deleted)
    {
        return false;
    }
    
    /*Now compare the child nodes linked by edges with the same output label.
     Since we are only dealing with observable FSMs, the output label
     uniquely determines the edge and the target node: all outputs
     have been generated from the SAME input.*/
    for (auto &e : treeNode1.children)
    {
        int y = e->getIO();
        bool yFound = false;
        
        for (auto &eOther : treeNode2.children)
        {
            if (y == eOther->getIO())
            {
                if (!(*e->getTarget() == *eOther->getTarget()))
                {
                    return false;
                }
                yFound = true;
                break;
            }
        }
        
        /*If otherNode does not have an outgoing edge labelled with y, the nodes differ.*/
        if (!yFound)
        {
            return false;
        }
        
    }
    return true;
}

TreeNode *TreeNode::add(const int x) {
    for (auto &e : children) {
        if (e->getIO() == x) {
            return e->getTarget();
        }
    }
    
    auto returnValue = new TreeNode();
    std::unique_ptr<TreeNode> tgt { returnValue };
    std::unique_ptr<TreeEdge> edge { new TreeEdge(x, std::move(tgt)) };
    add(std::move(edge));
    return returnValue;
}

void TreeNode::add(vector<int>::const_iterator lstIte, const vector<int>::const_iterator end)
{
    /*There may be no next list element, when this method is called*/
    if (lstIte == end)
    {
        return;
    }
    
    /*Which input is represented by the list iterator?*/
    int x = *lstIte++;
    
    for (auto &e : getChildren())
    {
        /*Is there already an edge labelled with this input?*/
        if (e->getIO() == x)
        {
            /*We do not need to extend the tree, but follow the existing edge*/
            TreeNode *nTgt = e->getTarget();
            nTgt->add(lstIte, end);
            return;
        }
    }
    
    /*No edge labelled with x exists for this node.
     Therefore one has to be created*/
    TreeNode *newNode = new TreeNode();
    std::unique_ptr<TreeEdge> newEdge { new TreeEdge(x, std::unique_ptr<TreeNode>(newNode)) };
    add(std::move(newEdge));
    newNode->add(lstIte, end);
}

void TreeNode::add(IOListContainer tcl)
{
    /*First delegate the work to the children*/
    for (auto &e : getChildren()) {
        TreeNode *nTgt = e->getTarget();
        nTgt->add(tcl);
    }
    
    /*Now append each input sequence in tcl to this node,
     using the special strategy of the add(lstIte) operation*/
    for (vector<int>& lst : tcl.getIOLists()) {
        add(lst.cbegin(), lst.cend());
    }
}


int TreeNode::tentativeAddToThisNode(vector<int>::const_iterator start,
                                     vector<int>::const_iterator stop) const {
    
    // If we have reached the end, the trace is fully contained in the tree
    if ( start == stop ) {
        return 0;
    }
    
    // If this is a node without children (i.e., a leaf),
    // we just have to extend the tree, without creating a new branch.
    if ( children.empty() ) {
        return 1;
    }
    
    // Now we have to check whether an existing edge
    // is labelled with inout *start
    int x = *start;
    for ( auto &e : children ) {
        if ( e->getIO() == x ) {
            return tentativeAddToThisNode(++start,stop);
        }
    }
    
    // Adding this trace requires a new branch in the tree,
    // this means, an additional test case.
    return 2;
}

int TreeNode::tentativeAddToThisNode(vector<int>::const_iterator start,
                                     vector<int>::const_iterator stop,
                                     TreeNode const *&n) const {
    
    n = this;
    
    // If we have reached the end, the trace is fully contained in the tree
    if ( start == stop ) {
        return 0;
    }
    
    // If this is a node without children (i.e., a leaf),
    // we just have to extend the tree, without creating a new branch.
    if ( children.empty() ) {
        return 1;
    }
    
    // Now we have to check whether an existing edge
    // is labelled with inout *start
    int x = *start;
    for ( auto const &e : children ) {
        if ( e->getIO() == x ) {
            TreeNode *next = e->getTarget();
            return next->tentativeAddToThisNode(++start,stop,n);
        }
    }
    
    // Adding this trace requires a new branch in the tree,
    // this means, an additional test case.
    return 2;
}



void TreeNode::addToThisNode(IOListContainer tcl)
{
    /*Append each input sequence in tcl to this node,
     using the special strategy of the add(lstIte) operation*/
    for (vector<int> const &lst : tcl.getIOLists())
    {
        add(lst.cbegin(), lst.cend());
    }
}

void TreeNode::addToThisNode(const vector<int> &lst)
{
    add(lst.cbegin(), lst.cend());
}

TreeNode *TreeNode::after(vector<int>::const_iterator lstIte, const vector<int>::const_iterator end)
{
    if (lstIte != end)
    {
        int x = *lstIte++;
        
        for (auto &e : getChildren())
        {
            if (e->getIO() == x)
            {
                return e->getTarget()->after(lstIte, end);
            }
        }
        
        /*Could not find an edge labelled by x*/
        return nullptr;
    }
    
    /*This is the last node reached by the input trace
     represented by iterator trIte. We have processed the
     whole input trace and can therefore return this
     as the final node.*/
    return this;
}

void TreeNode::calcSize(size_t& theSize) {
    theSize++;
    for ( auto &t : children) {
        t->getTarget()->calcSize(theSize);
    }
}



void TreeNode::traverse(vector<int>& v,
                        std::vector<std::vector<int>> &ioll) {
                        //IOListContainer::IOListBaseType &ioll) {
    // traverse all edges to child nodes
    for ( auto &e : children ) {
        int io = e->getIO();
        TreeNode *n = e->getTarget();
        v.push_back(io);
        
        n->traverse(v,ioll);
        
        // Pop the last element in v
        v.pop_back();
    }
    
    // add v to vector of I/O-lists
    ioll.push_back(v);
}
