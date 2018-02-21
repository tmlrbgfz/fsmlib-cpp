/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 *
 * Licensed under the EUPL V.1.1
 */
#include <deque>

#include "fsm/FsmNode.h"
#include "fsm/FsmTransition.h"
#include "fsm/OutputTrace.h"
#include "fsm/OFSMTable.h"
#include "fsm/DFSMTableRow.h"
#include "fsm/PkTable.h"
#include "trees/TreeEdge.h"
#include "trees/TreeNode.h"
#include "trees/OutputTree.h"
#include "trees/Tree.h"
#include "trees/TreeNode.h"
#include "trees/IOListContainer.h"
#include "interface/FsmPresentationLayer.h"

using namespace std;

FsmNode::FsmNode(const int id, FsmPresentationLayer const *presentationLayer)
: id(id),
visited(false),
color(white),
presentationLayer(presentationLayer),
derivedFromPair(nullptr, nullptr),
isInitialNode(false)
{
    
}

FsmNode::FsmNode(const int id, const string & name,
                 FsmPresentationLayer const *presentationLayer)
: FsmNode(id, presentationLayer)
{
    this->name = name;
}

void FsmNode::addTransition(std::unique_ptr<FsmTransition> &&transition)
{
    
    // Do not accept another transition with the same label and the
    // the same target node
    for ( auto &tr : transitions ) {
        if ( tr->getTarget() == transition->getTarget()
            and
            tr->getLabel() == transition->getLabel() ) {
            return;
        }
    }
    
    transitions.emplace_back(std::move(transition));
}

std::vector<std::unique_ptr<FsmTransition>>& FsmNode::getTransitions()
{
    return transitions;
}

std::vector<std::unique_ptr<FsmTransition>> const& FsmNode::getTransitions() const {
    return transitions;
}

int FsmNode::getId() const
{
    return id;
}

std::string FsmNode::getName() const
{
    return presentationLayer->getStateId(id, name);
}

bool FsmNode::hasBeenVisited() const
{
    return visited;
}

void FsmNode::setVisited()
{
    visited = true;
}

void FsmNode::setUnvisited() {
    visited = false;
}

void FsmNode::setPair(FsmNode *l, FsmNode *r)
{
    derivedFromPair = std::pair<FsmNode*, FsmNode*>(l, r);
}

void FsmNode::setPair(std::pair<FsmNode*, FsmNode*> const &p)
{
    derivedFromPair = p;
}

bool FsmNode::isDerivedFrom(std::pair<FsmNode*, FsmNode*> const &p) const
{
    return derivedFromPair == p;
}

std::pair<FsmNode*, FsmNode*> FsmNode::getPair() const
{
    return derivedFromPair;
}

FsmNode* FsmNode::apply(const int e, OutputTrace & o) const
{
    InputTrace inputTrace(presentationLayer->clone());
    inputTrace.add(e);
    auto applyResult = apply(inputTrace);
    auto leaves = applyResult.first.getLeaves();
    auto treeNode = leaves.front();
    auto path = treeNode->getPath();
    OutputTrace trace(path, presentationLayer->clone());
    o = std::move(trace);
    return applyResult.second[treeNode];
}

OutputTree FsmNode::apply(InputTrace const &itrc, bool markAsVisited) {
    auto applyResult = apply(itrc);
    if(markAsVisited) {
        this->setVisited();
        for(auto &mapElement : applyResult.second) {
            if(mapElement.second != nullptr) {
                mapElement.second->setVisited();
            }
        }
    }
    return applyResult.first;
}

std::pair<OutputTree, std::unordered_map<TreeNode*, FsmNode*>> FsmNode::apply(const InputTrace& itrc) const
{
    deque<TreeNode*> tnl;
    unordered_map<TreeNode*, FsmNode*> t2f;
    
    OutputTree ot = OutputTree(itrc, presentationLayer->clone());
    TreeNode *root = ot.getRoot();
    
    //Cannot use `this` here as it is const.
    t2f[root] = nullptr;
    
    for (auto it = itrc.cbegin(); it != itrc.cend(); ++ it)
    {
        int x = *it;
        
        vector< TreeNode* > vaux = ot.getLeaves();
        
        for ( auto n : vaux ) {
            tnl.push_back(n);
        }
        
        while (!tnl.empty())
        {
            TreeNode* thisTreeNode = tnl.front();
            tnl.pop_front();
            
            FsmNode *thisState = t2f.at(thisTreeNode);
            auto const &theseTransitions = [this](FsmNode *ptr)->std::vector<std::unique_ptr<FsmTransition>> const &{
                if(ptr != nullptr) {
                    return ptr->getTransitions();
                } else {
                    return this->getTransitions();
                }
            }(thisState);
            
            for (auto const &tr : theseTransitions)
            {
                if (tr->getLabel()->getInput() == x)
                {
                    int y = tr->getLabel()->getOutput();
                    FsmNode *tgtState = tr->getTarget();
                    std::unique_ptr<TreeNode> tgtNode { new TreeNode() };
                    auto tgtNodePtr = tgtNode.get();
                    std::unique_ptr<TreeEdge> te { new TreeEdge(y, std::move(tgtNode)) };
                    thisTreeNode->add(std::move(te));
                    t2f[tgtNodePtr] = tgtState;
                }
            }
        }
    }
    return std::make_pair(std::move(ot), std::move(t2f));
}

std::unordered_set<FsmNode*> FsmNode::after(std::vector<int> const &itrc)
{
    unordered_set<FsmNode*> nodeSet;
    nodeSet.insert(this);

    for (auto it = itrc.begin(); it != itrc.end(); ++it)
    {
        int x = *it;
        std::unordered_set<FsmNode*> newNodeSet;

        for (FsmNode *n : nodeSet)
        {
            std::unordered_set<FsmNode*> ns = n->afterAsSet(x);
            newNodeSet.insert(ns.begin(), ns.end());
        }
        nodeSet = newNodeSet;
    }
    return nodeSet;
}


std::unordered_set<FsmNode*> FsmNode::after(InputTrace const &itrc)
{
    return after(itrc.get());
    }
    
std::unordered_set<FsmNode*> FsmNode::after(TraceSegment const &seg) {
    return after(seg.get());
}

vector<FsmNode*> FsmNode::after(const int x) const
{
    vector<FsmNode*> lst;
    for (auto const &tr : transitions)
    {
        if (tr->getLabel()->getInput() == x)
        {
            lst.push_back(tr->getTarget());
        }
    }
    return lst;
}

std::unordered_set<FsmNode*> FsmNode::afterAsSet(const int x) const
{
    std::vector<FsmNode*> value = after(x);
    return std::unordered_set<FsmNode*>(value.begin(), value.end());
}

void FsmNode::setColor(const int pcolor)
{
    color = pcolor;
}

int FsmNode::getColor() const
{
    return color;
}

shared_ptr<DFSMTableRow> FsmNode::getDFSMTableRow(const int maxInput) const
{
    shared_ptr<DFSMTableRow> r = make_shared<DFSMTableRow>(id, maxInput);
    
    IOMap& io = r->getioSection();
    I2PMap& i2p = r->geti2postSection();
    
    for (auto const &tr : transitions)
    {
        int x = tr->getLabel()->getInput();
        
        /*Check whether transitions from this state are nondeterministic.
         This is detected when detecting a second transition triggered
         by the same input. In this case we cannot calculate a  DFSMTableRow.*/
        if (io.at(x) >= 0)
        {
            cout << "Cannot calculated DFSM table for nondeterministic FSM." << endl;
            return nullptr;
        }
        
        io[x] = tr->getLabel()->getOutput();
        i2p[x] = tr->getTarget()->getId();
    }
    return r;
}

bool FsmNode::distinguished(FsmNode const *otherNode, const vector<int>& iLst) const
{
    InputTrace itr = InputTrace(iLst, presentationLayer->clone());
    OutputTree ot1 = apply(itr).first;
    OutputTree ot2 = otherNode->apply(itr).first;
    
    return !(ot1 == ot2);
}

std::unique_ptr<InputTrace> FsmNode::distinguished(FsmNode const *otherNode, Tree const *w) const
{
    IOListContainer iolc = w->getIOLists();
    IOListContainer::IOListBaseType inputLists = iolc.getIOLists();
    
    for (vector<int>& iLst : inputLists)
    {
        if (distinguished(otherNode, iLst))
        {
            return std::unique_ptr<InputTrace>(new InputTrace(iLst, presentationLayer->clone()));
        }
    }
    return {};
}

InputTrace FsmNode::calcDistinguishingTrace(FsmNode *otherNode,
                                            const vector<shared_ptr<PkTable>>& pktblLst,
                                            const int maxInput)
{
    /*Determine the smallest l >= 1, such that this and otherNode are
     distinguished by P_l, but not by P_(l-1).
     Note that table P_n is found at pktblLst.get(n-1)*/
    unsigned int l;
    for (l = 1; l <= pktblLst.size(); ++ l)
    {
        /*Two nodes are distinguished by a Pk-table, if they
         reside in different Pk-table classes.*/
        shared_ptr<PkTable> pk = pktblLst.at(l - 1);
        if (pk->getClass(this->getId()) != pk->getClass(otherNode->getId()))
        {
            break;
        }
    }
    
    FsmNode *qi = this;
    FsmNode *qj = otherNode;
    
    InputTrace itrc = InputTrace(presentationLayer->clone());
    
    for (int k = 1; l - k > 0; ++ k)
    {
        bool foundNext = false;
        shared_ptr<PkTable> plMinK = pktblLst.at(l - k - 1);
        /*Determine input x such that qi.after(x) is distinguished
         from qj.after(x) in plMinK*/
        
        for (int x = 0; x <= maxInput; ++ x)
        {
            /*We are dealing with completely defined DFSMs,
             so after() returns an ArrayList containing exactly
             one element.*/
            FsmNode *qiNext = qi->after(x).front();
            FsmNode *qjNext = qj->after(x).front();
            
            if ( plMinK->getClass(qiNext->getId()) != plMinK->getClass(qjNext->getId()) )
            {
                qi = qiNext;
                qj = qjNext;
                itrc.add(x);
                foundNext = true;
                break;
            }
        }
        
        if ( not foundNext ) {
            cerr << "ERROR: inconsistency 1 detected when deriving distinguishing trace from Pk-Tables" << endl;
        }
        
    }
    
    /*Now the case l == k. qi and qj must be distinguishable by at least
     one input*/
    bool foundLast = false;
    for (int x = 0; x <= maxInput; ++ x) {
        
        OutputTrace oti = OutputTrace(presentationLayer->clone());
        OutputTrace otj = OutputTrace(presentationLayer->clone());
        qi->apply(x, oti);
        qj->apply(x, otj);
        if (oti.get().front() != otj.get().front())
        {
            itrc.add(x);
            foundLast = true;
            break;
        }
    }
    
    if ( not foundLast ) {
        cerr << "ERROR: inconsistency 2 detected when deriving distinguishing trace from Pk-Tables" << endl;
    }
    
    return itrc;
}

InputTrace FsmNode::calcDistinguishingTrace(FsmNode const *otherNode,
                                            const vector<shared_ptr<OFSMTable>>& ofsmTblLst,
                                            const int maxInput,
                                            const int maxOutput) const
{
    InputTrace itrc = InputTrace(presentationLayer->clone());
    int q1 = this->getId();
    int q2 = otherNode->getId();
    
    /*Now we know that this and otherNode are NOT distinguished by OFSM-Table-0.
     Determine the smallest l >= 1, such that this and otherNode are
     distinguished by OFSM-Table l, but not by OFSM-table (l-1).
     Note that table OFSM-table n is found at ofsmTblLst.get(n).*/
    unsigned int l;
    for (l = 1; l < ofsmTblLst.size(); ++ l)
    {
        /*Two nodes are distinguished by a OFSM-table, if they
         reside in different OFSM-table classes.*/
        shared_ptr<OFSMTable> ot = ofsmTblLst.at(l);
        if (ot->getS2C().at(q1) != ot->getS2C().at(q2))
        {
            break;
        }
    }
    
    for (int k = 1; l - k > 0; ++ k)
    {
        shared_ptr<OFSMTable> ot = ofsmTblLst.at(l - k);
        
        /*Determine IO x/y such that qi.after(x/y) is distinguished
         from qj.after(x/y) in ot*/
        for (int x = 0; x <= maxInput; ++ x)
        {
            for (int y = 0; y <= maxOutput; ++ y)
            {
                int q1Post = ot->get(q1, x, y);
                int q2Post = ot->get(q2, x, y);
                
                if (q1Post < 0 || q2Post < 0)
                {
                    continue;
                }
                
                if (ot->getS2C().at(q1Post) != ot->getS2C().at(q2Post))
                {
                    itrc.add(x);
                    
                    /*Set q1,q2 to their post-states under x/y*/
                    q1 = q1Post;
                    q2 = q2Post;
                    break;
                }
            }
        }
    }
    
    /*Now the case l == k. q1 and q2 must be distinguishable by at least
     one IO in OFSM-Table-0*/
    shared_ptr<OFSMTable> ot0 = ofsmTblLst.front();
    for (int x = 0; x <= maxInput; ++ x)
    {
        for (int y = 0; y <= maxOutput; ++ y)
        {
            if ( (ot0->get(q1, x, y) < 0 && ot0->get(q2, x, y) >= 0) or
                 (ot0->get(q1, x, y) >= 0 && ot0->get(q2, x, y) < 0))
            {
                itrc.add(x);
                return itrc;
            }
        }
    }
    return itrc;
}

bool FsmNode::isObservable() const
{
    
    for ( size_t t = 0; t < transitions.size(); t++ ) {
        
        auto lbl = transitions[t]->getLabel();
        
        for ( size_t other = t + 1; other < transitions.size(); other++ ) {
            auto otherLbl = transitions[other]->getLabel();
            if ( *lbl == *otherLbl ) return false;
        }
        
    }
    
    return true;
    
}

bool FsmNode::isDeterministic() const
{
    unordered_set<int> inputSet;
    
    /*Check if more than one outgoing transition
     is labelled with the same input value*/
    for (auto const &tr : transitions)
    {
        int inp = tr->getLabel()->getInput();
        if (!inputSet.insert(inp).second)
        {
            return false;
        }
    }
    return true;
}

ostream & operator<<(ostream & out, const FsmNode & node)
{
    for (auto &tr : node.transitions)
    {
        out << *tr << endl;
    }
    return out;
}

bool operator==(FsmNode const & node1, FsmNode const & node2)
{
    if (node1.id == node2.id)
    {
        return true;
    }
    return false;
}

void FsmNode::accept(FsmVisitor& v) {
    v.visit(*this);
}

void FsmNode::accept(FsmVisitor& v,
                     std::deque< FsmNode* >& bfsq){
    
    setVisited();
    v.visit(*this);
    
    for ( auto const &t : transitions ) {
        t->accept(v);
        t->getTarget()->accept(v);
        if ( not t->getTarget()->hasBeenVisited() ) {
            bfsq.push_back(t->getTarget());
        }
    }
    
}
