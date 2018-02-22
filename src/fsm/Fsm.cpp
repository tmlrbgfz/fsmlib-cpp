/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 *
 * Licensed under the EUPL V.1.1
 */

#include <chrono>

#include "fsm/Dfsm.h"
#include "fsm/Fsm.h"
#include "fsm/FsmNode.h"
#include "fsm/FsmTransition.h"
#include "fsm/InputTrace.h"
#include "fsm/OFSMTable.h"
#include "sets/HittingSet.h"
#include "trees/TreeNode.h"
#include "trees/OutputTree.h"
#include "trees/Tree.h"
#include "trees/IOListContainer.h"
#include "trees/TestSuite.h"


using namespace std;

std::unique_ptr<FsmNode> Fsm::newNode(const int id, std::pair<FsmNode*, FsmNode*> const &p,
                                      FsmPresentationLayer *pl) const
{
    FsmNode *n { new FsmNode(id, pl->getStateId(id,"")) };
    n->setDerivedFrom(p.first, p.second);
    return std::unique_ptr<FsmNode>(n);
}

bool Fsm::contains(deque<pair<FsmNode*, FsmNode*>> const &lst,
                   pair<FsmNode*, FsmNode*> const &p) const {
    return std::find(lst.begin(), lst.end(), p) != lst.end();
}

bool Fsm::contains(vector<FsmNode*> const &lst, FsmNode const *n) const {
    return std::any_of(lst.begin(), lst.end(), [n](FsmNode const *node)->bool{
        return node->isDerivedFrom(n->getDerivedFrom());
    });
}

FsmNode * Fsm::findp(vector<FsmNode*> const &lst,
                               pair<FsmNode*, FsmNode*> const &p) const {
    std::vector<FsmNode*> vec;
    vec.push_back(p.first);
    vec.push_back(p.second);
    auto iter = std::find_if(lst.begin(), lst.end(), [&vec](FsmNode const *node)->bool{
        return node->isDerivedFrom(vec);
    });
    if(iter != lst.end()) {
        return *iter;
    }
    return nullptr;
}

void Fsm::parseLine(const string & line)
{
    stringstream ss(line);
    
    int source;
    int input;
    int output;
    int target;
    ss >> source;
    ss >> input;
    ss >> output;
    ss >> target;
    
    if (source < 0 || static_cast<int> (nodes.size()) <= source)
    {
        return;
    }
    if (target < 0 || static_cast<int> (nodes.size()) <= target)
    {
        return;
    }
    if (input < 0 || maxInput < input)
    {
        return;
    }
    if (output < 0 || maxOutput < output)
    {
        return;
    }
    
    /*First node number occurring in the file defines the initial state*/
    if (initStateIdx < 0)
    {
        initStateIdx = source;
    }
    
    if (currentParsedNode == nullptr)
    {
        currentParsedNode = new FsmNode(source, name);
        nodes.at(source) = std::unique_ptr<FsmNode>(currentParsedNode);
        nodes.at(source)->setFsm(this);
    }
    else if (currentParsedNode->getId() != source && nodes[source] == nullptr)
    {
        currentParsedNode = new FsmNode(source, name);
        nodes.at(source) = std::unique_ptr<FsmNode>(currentParsedNode);
        nodes.at(source)->setFsm(this);
    }
    else if (currentParsedNode->getId() != source)
    {
        currentParsedNode = nodes.at(source).get();
    }
    
    if (nodes.at(target) == nullptr)
    {
        nodes.at(target) = std::unique_ptr<FsmNode>(new FsmNode(target, name));
        nodes.at(target)->setFsm(this);
    }
    
    FsmLabel *lbl = new FsmLabel(input, output, presentationLayer.get());
    std::unique_ptr<FsmTransition> transition { new FsmTransition(currentParsedNode, nodes.at(target).get(), std::unique_ptr<FsmLabel>(lbl)) };
    currentParsedNode->addTransition(std::move(transition));
}

void Fsm::parseLineInitial (const string & line)
{
    stringstream ss(line);
    
    int source;
    int input;
    int output;
    int target;
    ss >> source;
    ss >> input;
    ss >> output;
    ss >> target;
    
    if ( source > maxState ) maxState = source;
    if ( target > maxState ) maxState = target;
    if ( input > maxInput ) maxInput = input;
    if ( output > maxOutput ) maxOutput = output;
    
}


void Fsm::readFsm(const string & fname)
{
    
    // Read the FSM file first to determine maxInput, maxOutput, maxState
    readFsmInitial(fname);
    
    // Create the node vector, but first with null-nodes only
    for ( int n = 0; n <= maxState; n++ ) {
        nodes.emplace_back();
    }
    
    // Now read FSM file again to specify the FSM nodes and their transitions
    
    /* Mark that the initial state has not yet been determined
     (will be done in parseLine()) */
    initStateIdx = -1;
    ifstream inputFile(fname);
    if (inputFile.is_open())
    {
        string line;
        while (getline(inputFile, line))
        {
            parseLine(line);
        }
        inputFile.close();
        
    }
    else
    {
        std::cerr << "Unable to open input file" << endl;
        exit(EXIT_FAILURE);
    }
    
}

void Fsm::readFsmInitial (const string & fname)
{
    
    
    initStateIdx = -1;
    ifstream inputFile (fname);
    if (inputFile.is_open())
    {
        string line;
        while (getline (inputFile, line))
        {
            parseLineInitial (line);
        }
        inputFile.close ();
    }
    else
    {
        cout << "Unable to open input file" << endl;
        exit(EXIT_FAILURE);
    }
    
}


string Fsm::labelString(unordered_set<FsmNode*> const &lbl) const
{
    string s = "{ ";
    
    bool isFirst = true;
    for (FsmNode *n : lbl)
    {
        if (!isFirst)
        {
            s += ",";
        }
        isFirst = false;
        s += n->getName() + "(" + to_string(n->getId()) + ")";
    }
    
    s += " }";
    return s;
}

Fsm::Fsm() { }

Fsm::Fsm(const Fsm& other) {
    
    name = other.name;
    currentParsedNode = nullptr;
    maxInput = other.maxInput;
    maxOutput = other.maxOutput;
    maxState = other.maxState;
    initStateIdx = other.initStateIdx;
    characterisationSet = nullptr;
    minimal = other.minimal;
    presentationLayer = other.presentationLayer->clone();
    
    for ( int n = 0; n <= maxState; n++ ) {
        nodes.emplace_back(new FsmNode(n,name));
        nodes.back()->setFsm(this);
    }
    
    // Now add transitions that correspond exactly to the transitions in
    // this FSM
    for ( int n = 0; n <= maxState; n++ ) {
        auto theNewFsmNodeSrc = nodes[n].get();
        auto theOldFsmNodeSrc = other.nodes[n].get();
        for ( auto &tr : theOldFsmNodeSrc->getTransitions() ) {
            int tgtId = tr->getTarget()->getId();
            std::unique_ptr<FsmLabel> newLbl { new FsmLabel(*tr->getLabel()) };
            std::unique_ptr<FsmTransition> transition { new FsmTransition(theNewFsmNodeSrc,nodes[tgtId].get(),std::move(newLbl)) };
            theNewFsmNodeSrc->addTransition(std::move(transition));
        }
    }
    
    // Mark the initial node
    nodes.at(initStateIdx)->markAsInitial();
    
}

Fsm::Fsm(std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
:
name(""),
currentParsedNode(nullptr),
maxInput(-1),
maxOutput(-1),
maxState(-1),
initStateIdx(-1),
characterisationSet(nullptr),
minimal(Maybe),
presentationLayer(std::move(presentationLayer))
{
    
}

Fsm::Fsm(const string& fname,
         std::unique_ptr<FsmPresentationLayer> &&presentationLayer,
         const string& fsmName)
:
name(fsmName),
currentParsedNode(nullptr),
maxInput(-1),
maxOutput(-1),
maxState(-1),
characterisationSet(nullptr),
minimal(Maybe),
presentationLayer(std::move(presentationLayer))
{
    readFsm(fname);
    if ( initStateIdx >= 0 ) nodes[initStateIdx]->markAsInitial();
    
}

Fsm::Fsm(const string & fname,
         const string & fsmName,
         const int maxNodes,
         const int maxInput,
         const int maxOutput,
         std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
:
name(fsmName),
currentParsedNode(nullptr),
maxInput(maxInput),
maxOutput(maxOutput),
maxState(maxNodes),
characterisationSet(nullptr),
minimal(Maybe),
presentationLayer(std::move(presentationLayer))
{
    
    for (int i = 0; i < maxNodes; ++ i)
    {
        nodes.emplace_back();
    }
    readFsm (fname);
    if ( initStateIdx >= 0 ) nodes[initStateIdx]->markAsInitial();

}

Fsm::Fsm(const string & fsmName,
         const int maxInput,
         const int maxOutput,
         std::vector<std::unique_ptr<FsmNode>> &&lst,
         std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
:
name(fsmName),
nodes(std::move(lst)),
currentParsedNode(nullptr),
maxInput(maxInput),
maxOutput(maxOutput),
maxState((int)(nodes.size()-1)),
initStateIdx(0),
characterisationSet(nullptr),
minimal(Maybe),
presentationLayer(std::move(presentationLayer))
{
    // reset all nodes as 'white' and 'unvisited'
    for ( auto &n : nodes ) {
        n->setColor(FsmNode::white);
        n->setUnvisited();
        n->setFsm(this);
    }
    
    nodes[initStateIdx]->markAsInitial();
}



void Fsm::dumpFsm(ofstream & outputFile) const
{
    for (unsigned int i = 0; i < nodes.size(); ++ i)
    {
        auto const &transitions = nodes.at(i)->getTransitions();
        for (unsigned int j = 0; j < transitions.size(); ++ j)
        {
            FsmTransition const *tr = transitions.at(j).get();
            outputFile << i << " "
            << tr->getLabel()->getInput()
            << " " << tr->getLabel()->getOutput()
            << " " << tr->getTarget()->getId();
            if (j < transitions.size() - 1 || i < nodes.size() - 1)
            {
                outputFile << endl;
            }
        }
    }
}



FsmNode * Fsm::getInitialState() const
{
    return nodes.size() > 0 ? nodes.at(initStateIdx).get() : nullptr;
}

void Fsm::setInitialState(FsmNode *node) {
    auto found = std::find_if(nodes.begin(), nodes.end(), [node](std::unique_ptr<FsmNode> const &elem){
        return elem.get() == node;
    });
    if(found != nodes.end()) {
        initStateIdx = found - nodes.begin();
    }
}

string Fsm::getName() const
{
    return name;
}

int Fsm::getMaxNodes() const
{
    return static_cast<int> (nodes.size());
}

int Fsm::getMaxInput() const
{
    return maxInput;
}

int Fsm::getMaxOutput() const
{
    return maxOutput;
}

std::vector<std::unique_ptr<FsmNode>> const &Fsm::getNodes() const {
    return nodes;
}

FsmPresentationLayer *Fsm::getPresentationLayer() const
{
    return presentationLayer.get();
}

int Fsm::getInitStateIdx() const
{
    return initStateIdx;
}

void Fsm::resetColor()
{
    for (auto &node : nodes) {
        node->setColor(FsmNode::white);
    }
}

void Fsm::toDot(const string & fname)
{
    ofstream out(fname + ".dot");
    out << *this;
    out.close();
}

Fsm Fsm::intersect(const Fsm & f) const
{
    // A list of node pairs which is used to
    // control the breath-first search (BFS)
    std::deque<std::pair<FsmNode*, FsmNode*>> nodeList;

    // A list of new FSM states, each state created from a pair of
    // this-nodes and f-nodes. At the end of this operation,
    // the new FSM will be created from this list.
    std::vector<FsmNode*> fsmInterNodes;
    int id = 0;
    
    // Initially, add the pair of initial this-node and f-node
    // into the BFS list.
    nodeList.push_back(std::pair<FsmNode*, FsmNode*>(getInitialState(), f.getInitialState()));
    
    // We need a new presentation layer. It has the same inputs and
    // outputs as this Fsm, but the state names will be pairs of
    // state names from this FSM and f
    vector<std::string> stateNames;
    std::unique_ptr<FsmPresentationLayer> newPl {
        new FsmPresentationLayer(presentationLayer->getIn2String(),
                                presentationLayer->getOut2String(),
                                stateNames) };
    
    // This is the BFS loop, running over the (this,f)-node pairs
    while (!nodeList.empty())
    {
        // Remove the head of the list and use p to refer to it
        // p refers to the SOURCE node pair, from where all
        // outgoing transitions are investigated in this loop cycle
        std::pair<FsmNode*, FsmNode*> p = nodeList.front();
        nodeList.pop_front();
        
        // current node of this FSM
        FsmNode *myCurrentNode = p.first;
        
        // current node of the f-FSM
        FsmNode *theirCurrentNode = p.second;
        
        // Do we already have an FSM state for the new FSM
        // stored in fsmInterNodes, which is associated
        // with the current pair p?
        FsmNode *nSource = findp(fsmInterNodes, p);
        
        if (nSource == nullptr)
        {
            
            // Set the node name as pair of the individual node names
            string newNodeName("(" + myCurrentNode->getName() + "," +
                               theirCurrentNode->getName() + ")");
            
            // Register node name in new presentation layer
            newPl->addState2String(newNodeName);
            
            // We create the new FSM state associated with p:
            // nSource is created from the state
            // pair(myCurrentNode,theirCurrentNode)
            // which is identified by p.
            nSource = newNode(id++, p, newPl.get()).release();
            fsmInterNodes.push_back(nSource);
            
        }
        
        // Mark this node: now all of its outgoing transitions are constructed
        nSource->setVisited();
        
        // Loop over all transitions emanating from myCurrentNode
        for (auto &tr : myCurrentNode->getTransitions())
        {
            // Loop over all transitions emanating from theirCurrentNode
            for (auto &trOther : theirCurrentNode->getTransitions())
            {
                /* If tr and trOther have identical labels, we can create a
                   transition for the new FSM to be created.
                   The transition has source node (myCurrentNode,theirCurrentNode)
                   and label tr.getLabel(), which is the same as
                   the label associated with the other transition,
                   and target node (tr.getTarget(),trOther.getTarget()),
                   which is the pair of the target nodes
                   of each transition.*/
                
                if (*tr->getLabel() == *trOther->getLabel())
                {
                    
                    // New target node represented as a pair (this-node,f-node)
                    auto pTarget = std::pair<FsmNode*, FsmNode*>(tr->getTarget(), trOther->getTarget());
                    
                    // If the target node does not yet exist in the list
                    // of state for the new FSM, then create it now
                    FsmNode *nTarget = findp(fsmInterNodes, pTarget);
                    if (nTarget == nullptr)
                    {
                        // Set the node name as pair of the individual node names
                        string newNodeName("(" + tr->getTarget()->getName() +
                                           "," +
                                           trOther->getTarget()->getName() + ")");
                        
                        // Register node name in new presentation layer
                        newPl->addState2String(newNodeName);
                        
                        nTarget = newNode(id++, pTarget, newPl.get()).release();
                        fsmInterNodes.push_back(nTarget);
                    }
                    
                    // Add transition from nSource to nTarget
                    std::unique_ptr<FsmTransition> transition { new FsmTransition(nSource,
                                                                                  nTarget,
                                                                                  tr->getLabel()->clone()) };
                    nSource->addTransition(std::move(transition));
                    
                    /* Conditions for insertion of the target pair
                       into the nodeList:
                     1. the target node corresponding to the pair
                        has not yet been processed
                        (that is,  nTarget.hasBeenVisited() == false)
                     2. The target pair is not already entered into the nodeList
                     */
                    if (not (nTarget->hasBeenVisited() or
                             contains(nodeList, pTarget)))
                    {
                        nodeList.push_back(pTarget);
                    }
                }
            }
        }
    }
    
    newPl->dumpState(cout);
    std::vector<std::unique_ptr<FsmNode>> newNodes;
    std::transform(fsmInterNodes.begin(), fsmInterNodes.end(), std::back_inserter(newNodes), [](FsmNode * const &node){
        return std::unique_ptr<FsmNode>(node);
    });
    
    return Fsm(f.getName(), maxInput, maxOutput, std::move(newNodes), newPl->clone());
}

std::unique_ptr<Tree> Fsm::getStateCover()
{
    resetColor();
    std::deque<FsmNode*> bfsLst;
    unordered_map<FsmNode*, TreeNode*> f2t;
    
    std::unique_ptr<Tree> scov { new Tree(presentationLayer->clone()) };
    TreeNode *root = scov->getRoot();
    
    FsmNode *initState = getInitialState();
    initState->setColor(FsmNode::grey);
    bfsLst.push_back(initState);
    f2t[initState] = root;
    
    while (!bfsLst.empty())
    {
        FsmNode *thisNode = bfsLst.front();
        bfsLst.pop_front();
        TreeNode *currentTreeNode = f2t[thisNode];
        
        for (int x = 0; x <= maxInput; ++x)
        {
            for (FsmNode *tgt : thisNode->after(x))
            {
                if (tgt->getColor() == FsmNode::white)
                {
                    tgt->setColor(FsmNode::grey);
                    TreeNode *itn = currentTreeNode->add(x);
                    bfsLst.push_back(tgt);
                    f2t[tgt] = itn;
                }
            }
        }
        thisNode->setColor(FsmNode::black);
    }
    resetColor();
    return std::move(scov);
}

std::unique_ptr<Tree> Fsm::getTransitionCover()
{
    std::unique_ptr<Tree> scov = getStateCover();
    resetColor();
    
    IOListContainer::IOListBaseType tlst;
    
    for (int x = 0; x <= maxInput; ++ x)
    {
        vector<int> l;
        l.push_back(x);
        tlst.push_back(l);
    }
    
    IOListContainer tcl = IOListContainer(tlst, presentationLayer->clone());
    
    //TODO: Shouldn't this be filtered for traces that actually exist?
    scov->add(tcl);
    
    return std::move(scov);
}

OutputTree Fsm::apply(const InputTrace & itrc, bool markAsVisited)
{
    return getInitialState()->apply(itrc,markAsVisited);
}

Fsm Fsm::transformToObservableFSM() const
{
    
    // List to be filled with the new states to be created
    // for the observable FSM
    vector<FsmNode*> nodeLst;
    
    // Breadth first search list, containing the
    // new FSM nodes, still to be processed by the algorithm
    vector<FsmNode*> bfsLst;

    // Map a newly created node to the set of nodes from the
    // original FSM, comprised in the new FSM state
    unordered_map<FsmNode*, unordered_set<FsmNode*>> node2NodeLabel;
    
    // Set of nodes from the original FSM, comprised in the
    // current new node
    unordered_set<FsmNode*> theNodeLabel;
    
    // For the first step of the algorithm, the initial
    // state of the original FSM is the only state comprised
    // in the initial state of the new FSM
    theNodeLabel.insert(getInitialState());

    
    // Create a new presentation layer which has
    // the same names for the inputs and outputs as
    // the old presentation layer, but still an EMPTY vector
    // of node names.
    vector<string> obsState2String;
    std::unique_ptr<FsmPresentationLayer> obsPl {
        new FsmPresentationLayer(presentationLayer->getIn2String(),
                                presentationLayer->getOut2String(),
                                obsState2String) };
    
    
    // id to be taken for the next state of the new
    // observable FSM to be created.
    int id = 0;
    
    // The initial state of the new FSM is labelled with
    // the set containing just the initial state of the old FSM
    string nodeName = labelString(theNodeLabel);
    FsmNode *q0 = new FsmNode(id++, nodeName);
    nodeLst.push_back(q0);
    bfsLst.push_back(q0);
    node2NodeLabel[q0] = theNodeLabel;
    
    // The node label is added to the presentation layer,
    // as name of the initial state
    obsPl->addState2String(nodeName);
    
    // Loop while there is at least one node in the BFS list.
    // Initially, the list contains just the new initial state
    // of the new FSM.
    while (!bfsLst.empty())
    {
        // Pop the first node from the list
        FsmNode *q = bfsLst.front();
        bfsLst.erase(bfsLst.begin());
        
        // Nested loop over all input/output labels that
        // might be associated with one or more outgoing transitions
        for (int x = 0; x <= maxInput; ++ x)
        {
            for (int y = 0; y <= maxOutput; ++ y)
            {
                // This is the transition label currently processed
                std::unique_ptr<FsmLabel> lbl { new FsmLabel(x, y, obsPl.get()) };
                
                // Clear the set of node labels that may
                // serve as node name for the target node to be
                // created (or already exists). This target node
                // comprises all nodes of the original FSM that
                // can be reached from q under a transition labelled
                // with lbl
                theNodeLabel.clear();
                
                // Loop over all nodes of the original FSM which
                // are comprised by q
                for (FsmNode *n : node2NodeLabel.at(q))
                {
                    // For each node comprised by q, check
                    // its outgoing transitions, whether they are
                    // labelled by lbl
                    for (auto &tr : n->getTransitions())
                    {
                        if (*tr->getLabel() == *lbl)
                        {
                            // If so, insert the target node
                            // into the node label set
                            theNodeLabel.insert(tr->getTarget());
                        }
                    }
                }
                
                // Process only non-empty label sets.
                // An empty label set means that no transition labelled
                // by lbl exists.
                if (!theNodeLabel.empty())
                {
                    
                    FsmNode *tgtNode = nullptr;
                    
                    // Loop over the node-2-label map and check
                    // if there exists already a node labelled
                    // by theNodeLabel. If this is the case,
                    // it will become the target node reached from q
                    // under lbl
                    for ( auto u : node2NodeLabel ) {
                        
                        if ( u.second == theNodeLabel ) {
                            tgtNode = u.first;
                            break;
                        }
                        
                    }
                    
                    if (tgtNode == nullptr)
                    {
                        // We need to create a new target node, to be reached
                        // from q under lbl
                        nodeName = labelString(theNodeLabel);
                        tgtNode = new FsmNode(id++, nodeName);
                        nodeLst.push_back(tgtNode);
                        bfsLst.push_back(tgtNode);
                        node2NodeLabel[tgtNode] = theNodeLabel;
                        obsPl->addState2String(nodeName);
                    }
                    
                    // Create the transition from q to tgtNode
                    std::unique_ptr<FsmTransition> transition { new FsmTransition(q, tgtNode, std::move(lbl)) };
                    q->addTransition(std::move(transition));
                }
            }
        }
    }

    std::vector<std::unique_ptr<FsmNode>> newNodes;
    std::transform(nodeLst.begin(), nodeLst.end(), std::back_inserter(newNodes), [](FsmNode * const &node){
        return std::unique_ptr<FsmNode>(node);
    });
    return Fsm(name + "_O", maxInput, maxOutput, std::move(newNodes), obsPl->clone());
}

bool Fsm::isObservable() const {
    return std::all_of(nodes.begin(), nodes.end(), [](std::unique_ptr<FsmNode> const &node){
        return node->isObservable();
    });
}

Minimal Fsm::isMinimal() const
{
    return minimal;
}

void Fsm::calcOFSMTables() {
    
    ofsmTableLst.clear();
    
    // Create the initial OFSMTable representing the FSM,
    //  where all FSM states belong to the same class
    shared_ptr<OFSMTable> tbl = make_shared<OFSMTable>(nodes, maxInput, maxOutput, presentationLayer.get());
    
    // Create all possible OFSMTables, each new one from its
    // predecessor, and add them to the ofsmTableLst
    while (tbl != nullptr)
    {
        ofsmTableLst.push_back(tbl);
        tbl = tbl->next();
    }
    
}

Fsm Fsm::minimiseObservableFSM()
{
    calcOFSMTables();

    // The last OFSMTable defined has classes corresponding to
    // the minimised FSM to be constructed*/
    shared_ptr<OFSMTable> tbl = ofsmTableLst.back();
    
    // Create the minimised FSM from tbl and return it
    Fsm fsm = tbl->toFsm(name + "_MIN");
    fsm.minimal = True;
    return fsm;
}

Fsm Fsm::minimise()
{
    
    vector<std::unique_ptr<FsmNode>> uNodes;
    removeUnreachableNodes(uNodes);
    
    if (!isObservable())
    {
        return transformToObservableFSM().minimiseObservableFSM();
    }
    
    return minimiseObservableFSM();
}

bool Fsm::isCharSet(Tree const *w) const
{
    for (unsigned int i = 0; i < nodes.size(); ++ i)
    {
        for (unsigned int j = i + 1; j < nodes.size(); ++ j)
        {
            if (nodes.at(i)->distinguished(nodes.at(j).get(), w) == nullptr)
            {
                return false;
            }
        }
    }
    return true;
}

void Fsm::minimiseCharSet(Tree const *w)
{
    IOListContainer wcnt = w->getIOLists();
    if (wcnt.size() <= 1)
    {
        return;
    }
    
    for (unsigned int i = 0; i < wcnt.getIOLists().size(); ++ i)
    {
        IOListContainer wcntNew = IOListContainer(wcnt);
        wcnt.getIOLists().erase(wcnt.getIOLists().begin() + i);
        
        std::unique_ptr<Tree> itr { new Tree(presentationLayer->clone()) };
        itr->addToRoot(wcntNew);
        if (isCharSet(itr.get()))
        {
            if (itr->getIOLists().size() < characterisationSet->getIOLists().size())
            {
                characterisationSet = itr->clone();
            }
        }
        minimiseCharSet(characterisationSet.get());
    }
}

IOListContainer Fsm::getCharacterisationSet()
{
   std::cout << "Calculating characterisation set." << std::endl;
    // Do we already have a characterisation set ?
    if ( characterisationSet != nullptr ) {
        return characterisationSet->getIOLists();
    }
    
    
    // We have to calculate teh chracterisation set from scratch
    if (!isObservable())
    {
        cout << "This FSM is not observable - cannot calculate the charactersiation set." << endl;
        exit(EXIT_FAILURE);
    }
    
    /*Call minimisation algorithm again for creating the OFSM-Tables*/
    minimise();
    
    /*Create an empty characterisation set as an empty InputTree instance*/
    std::unique_ptr<Tree> w { new Tree(presentationLayer->clone()) };
    
    /*Loop over all non-equal pairs of states.
     Calculate the state identification sets.*/
    for (unsigned int left = 0; left < nodes.size(); ++ left)
    {
        FsmNode *leftNode = nodes.at(left).get();
        
        for (unsigned int right = left + 1; right < nodes.size(); ++ right)
        {
            FsmNode *rightNode = nodes.at(right).get();
            
            /*Nothing to do if leftNode and rightNode are
             already distinguished by an element of w*/
            if (leftNode->distinguished(rightNode, w.get()) != nullptr)
            {
                continue;
            }
            
            /*We have to create a new input trace and add it to w, because
             leftNode and rightNode are not distinguished by the current
             input traces contained in w. */
            InputTrace i = leftNode->calcDistinguishingTrace(rightNode,
                                                             ofsmTableLst,
                                                             maxInput,
                                                             maxOutput);
            IOListContainer::IOListBaseType lli;
            lli.push_back(i.get());
            IOListContainer tcli = IOListContainer(lli, presentationLayer->clone());
            
            /*Insert this also into w*/
            w->addToRoot(tcli);
        }
    }
    
    /*Minimise and store characterisation set*/
    characterisationSet = std::move(w);
    //    minimiseCharSet(w);
    
    /*Wrap list of lists by an IOListContainer instance*/
    IOListContainer tcl = characterisationSet->getIOLists();
    
    return tcl;
}

void Fsm::calcStateIdentificationSets()
{
    if (!isObservable())
    {
        cout << "This FSM is not observable - cannot calculate the charactersiation set." << endl;
        exit(EXIT_FAILURE);
    }
    
    if (characterisationSet == nullptr)
    {
        cout << "Missing characterisation set - exit." << endl;
        exit(EXIT_FAILURE);
    }
    
    /*Create empty state identification sets for every FSM state*/
    stateIdentificationSets.clear();
    
    /*Identify W by integers 0..m*/
    IOListContainer wIC = characterisationSet->getIOLists();
    IOListContainer::IOListBaseType wLst = wIC.getIOLists();
    
    /*wLst.get(0) is identified with Integer(0),
     wLst.get(1) is identified with Integer(1), ...*/
    
    vector<vector<unordered_set<int>>> z;
    z.resize(nodes.size());
    for (unsigned int i = 0; i < nodes.size(); ++ i) {
        z.at(i).resize(nodes.size());
    }
    
    for (unsigned int i = 0; i < nodes.size(); ++ i)
    {
        FsmNode *iNode = nodes.at(i).get();
        
        for (unsigned int j = i + 1; j < nodes.size(); ++ j)
        {
            FsmNode *jNode = nodes.at(j).get();
            
            for (unsigned int u = 0; u < wLst.size(); ++ u)
            {
                vector<int> thisTrace = wLst.at(u);
                
                if (iNode->distinguished(jNode, thisTrace))
                {
                    z.at(i).at(j).insert(u);
                    z.at(j).at(i).insert(u);
                }
            }
        }
    }
    
    for (unsigned int i = 0; i < nodes.size(); ++ i)
    {
        vector<unordered_set<int>> iLst;
        for (unsigned int j = 0; j < nodes.size(); ++ j) {
            if (i != j) {
                iLst.push_back(z.at(i).at(j));
            }
        }
        
        /*Calculate minimal state identification set for
         FsmNode i*/
        HittingSet hs = HittingSet(iLst);
        unordered_set<int> h = hs.calcMinCardHittingSet();
        
        std::unique_ptr<Tree> iTree { new Tree(presentationLayer->clone()) };
        for (int u : h) {
            vector<int> lli = wLst.at(u);
            IOListContainer::IOListBaseType lllli;
            lllli.push_back(lli);
            iTree->addToRoot(IOListContainer(lllli, presentationLayer->clone()));
        }
        stateIdentificationSets.push_back(std::move(iTree));
        
    }
}



void Fsm::calcStateIdentificationSetsFast()
{
    if (!isObservable())
    {
        cout << "This FSM is not observable - cannot calculate the charactersiation set." << endl;
        exit(EXIT_FAILURE);
    }
    
    if (characterisationSet == nullptr)
    {
        cout << "Missing characterisation set - exit." << endl;
        exit(EXIT_FAILURE);
    }
    
    /*Create empty state identification sets for every FSM state*/
    stateIdentificationSets.clear();
    
    /*Identify W by integers 0..m*/
    IOListContainer wIC = characterisationSet->getIOLists();
    IOListContainer::IOListBaseType wLst = wIC.getIOLists();
    
    // Matrix indexed over nodes
    vector< vector<int> > distinguish;
    
    // Every node is associated with an IOListContainer
    // containing its distinguishing traces
    vector< IOListContainer > node2iolc;
    
    for (size_t i = 0; i < size(); ++ i) {
        node2iolc.push_back(IOListContainer(presentationLayer->clone()));
        vector<int> v;
        distinguish.push_back(v);
        for (size_t j = 0; j < size(); j++ ) {
            distinguish.at(i).push_back(-1);
        }
    }
    
    for (size_t i = 0; i < size(); ++ i) {
        
        int traceIdx = 0;
        for (auto trc : wLst) {
            
            bool complete = true;
            for ( size_t j = i+1; j < size(); j++ ) {
                if ( distinguish.at(i).at(j) == -1 ) {
                    
                    if (nodes[i]->distinguished(nodes[j].get(), trc))
                    {
                        distinguish.at(i).at(j) = traceIdx;
                        distinguish.at(j).at(i) = traceIdx;
                        Trace tr(trc,presentationLayer->clone());
                        node2iolc.at(i).add(tr);
                        node2iolc.at(j).add(tr);
                    }
                    else {
                        complete = false;
                    }
                    
                }
            }
            
            if ( complete ) break;
            traceIdx++;
        
        }
    }
    
    
    for (size_t i = 0; i < size(); ++ i) {
        std::unique_ptr<Tree> iTree { new Tree(presentationLayer->clone()) };
        iTree->addToRoot(node2iolc.at(i));
        stateIdentificationSets.push_back(std::move(iTree));
    }
}


void Fsm::appendStateIdentificationSets(Tree *Wp2) const
{
    IOListContainer cnt = Wp2->getIOLists();
    
    for (vector<int> lli : cnt.getIOLists())
    {
        InputTrace itrc = InputTrace(lli, presentationLayer->clone());
        
        /*Which are the target nodes reachable via input trace lli
         in this FSM?*/
        unordered_set<FsmNode*> tgtNodes = getInitialState()->after(itrc);
        
        for (FsmNode const *n : tgtNodes)
        {
            int nodeId = n->getId();
            
            /*Get state identification set associated with n*/
            auto const &wNodeId = stateIdentificationSets.at(nodeId);
            
            /*Append state identification set to Wp2 tree node
             reached after applying  itrc*/
            Wp2->addAfter(itrc, wNodeId->getIOLists());
        }
    }
}


IOListContainer Fsm::wMethod(const unsigned int numAddStates) {
    return transformToObservableFSM().minimise().wMethodOnMinimisedFsm(numAddStates);
}

IOListContainer Fsm::wMethodOnMinimisedFsm(const unsigned int numAddStates) {
    
    std::unique_ptr<Tree> iTree = getTransitionCover();
    
    if ( numAddStates > 0 ) {
        IOListContainer inputEnum = IOListContainer(maxInput,
                                                    1,
                                                    (int)numAddStates,
                                                    presentationLayer->clone());
        iTree->add(inputEnum);
    }
    
    
    IOListContainer w = getCharacterisationSet();
    iTree->add(w);
    
    return iTree->getIOLists();
    
}

IOListContainer Fsm::wpMethod(const unsigned int numAddStates) {
    std::unique_ptr<Tree> scov = getStateCover();
    std::unique_ptr<Tree> tcov = getTransitionCover();
    tcov->remove(scov.get());
    IOListContainer w = getCharacterisationSet();
    calcStateIdentificationSetsFast();
    
    std::unique_ptr<Tree> Wp1 = scov->clone();
    if (numAddStates > 0) {
        IOListContainer inputEnum = IOListContainer(maxInput, 1,
                                                    (int)numAddStates,
                                                    presentationLayer->clone());
        Wp1->add(inputEnum);
    }
    Wp1->add(w);
    
    std::unique_ptr<Tree> Wp2 = tcov->clone();
    if (numAddStates > 0) {
        IOListContainer inputEnum = IOListContainer(maxInput,
                                                    (int)numAddStates,
                                                    (int)numAddStates,
                                                    presentationLayer->clone());
        Wp2->add(inputEnum);
    }
    appendStateIdentificationSets(Wp2.get());

    Wp1->unionTree(Wp2.get());
    return Wp1->getIOLists();
}


IOListContainer Fsm::hsiMethod(const unsigned int numAddStates)
{

    if (!isObservable())
    {
        cout << "This FSM is not observable - cannot calculate the harmonized state identification set." << endl;
        exit(EXIT_FAILURE);
    }
    
    IOListContainer wSet = getCharacterisationSet();

    /* V.(Inputs from length 1 to m-n+1) */
    std::unique_ptr<Tree> hsi = getStateCover();
    IOListContainer inputEnum = IOListContainer(maxInput,
                                                    1,
                                                    (int)numAddStates + 1,
                                                    presentationLayer->clone());
    hsi->add(inputEnum);

    /* initialize HWi trees */
    std::vector<std::unique_ptr<Tree>> hwiTrees;
    for (unsigned i = 0; i < nodes.size(); i++)
    {
        std::unique_ptr<Tree> emptyTree { new Tree(presentationLayer->clone()) };
        hwiTrees.push_back(std::move(emptyTree));
    }

    /* Create harmonised state identification set for every FSM state.
     * For each pair of nodes i and j get one element
     * of the characterisation set that distinguishes the two nodes.
     * Add the distinguishing sequence to both HWi and HWj.
     */
    for (unsigned i = 0; i < nodes.size()-1; i++)
    {
        FsmNode *node1 = nodes[i].get();
        for (unsigned j = i+1; j < nodes.size(); j++)
        {
            FsmNode *node2 = nodes[j].get();
            bool distinguished = false;
            for (auto iolst : wSet.getIOLists())
            {
                if (node1->distinguished(node2, iolst)){
                    distinguished = true;
                    hwiTrees[i]->addToRoot(iolst);
                    hwiTrees[j]->addToRoot(iolst);
                    break;
                }
            }
            if (!distinguished) {
                cout << "[ERR] Found inconsistency when applying HSI-Method: FSM not minimal." << endl;
            }
        }
    }

    /* Append harmonised state identification sets */
    IOListContainer cnt = hsi->getIOLists();
    for (auto lli : cnt.getIOLists())
    {
        InputTrace itrc = InputTrace(lli, presentationLayer->clone());

        /*Which are the target nodes reachable via input trace lli
         in this FSM?*/
        unordered_set<FsmNode*> tgtNodes = getInitialState()->after(itrc);

        for (auto n : tgtNodes)
        {
            int nodeId = n->getId();

            /* harmonised state identification set associated with n*/
            Tree *hwNodeId = hwiTrees[nodeId].get();

            /* Append harmonised state identification set to hsi tree node
               reached after applying itrc */
            hsi->addAfter(itrc,hwNodeId->getIOLists());
        }
    }

    return hsi->getIOLists();
}

TestSuite Fsm::createTestSuite(IOListContainer testCases)
{
    IOListContainer::IOListBaseType tcLst = testCases.getIOLists();
    TestSuite theSuite;
    std::transform(tcLst.begin(), tcLst.end(), std::back_inserter(theSuite), 
                   [this](IOListContainer::IOListBaseType::value_type const &trace){
        return apply(InputTrace(trace, presentationLayer->clone()));
    });
    
    return theSuite;
}

bool Fsm::isCompletelyDefined() const
{
    bool cDefd = true;
    for (auto const &nn : nodes)
    {
        for (int x = 0; x <= maxInput; ++ x)
        {
            bool found = false;
            for (auto const &tr : nn->getTransitions())
            {
                if (tr->getLabel()->getInput() == x)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                cout << "Incomplete FSM : for state " << nn->getName() << " " << nn->getId() << ", input " << x << " does not have a transition." << endl;
                cDefd = false;
            }
        }
    }
    return cDefd;
}

bool Fsm::isDeterministic() const
{
    return std::all_of(nodes.begin(), nodes.end(), [](std::unique_ptr<FsmNode> const &node){
        return node->isDeterministic();
    });
}

void Fsm::setPresentationLayer(std::unique_ptr<FsmPresentationLayer> &&ppresentationLayer)
{
    presentationLayer = std::move(ppresentationLayer);
}

ostream & operator<<(ostream & out, const Fsm & fsm)
{
    out << "digraph g {" << endl << endl << "node [shape = circle]" << endl << endl;
    for (int i = 0; i < static_cast<int> (fsm.nodes.size()); ++ i)
    {
        if (i == fsm.initStateIdx)
        {
            out << endl << "node [shape = doublecircle]" << endl;
        }
        
        if (fsm.nodes.at(i) == nullptr)
        {
            continue;
        }
        string nodeName = (fsm.nodes.at(i)->getName().empty()) ? "s" : fsm.nodes.at(i)->getName();
        out << i << "[label=\"" << nodeName << "(" << fsm.nodes.at(i)->getId() << ")\"];" << endl;
        
        if (i == fsm.initStateIdx)
        {
            out << endl << "node [shape = ellipse]" << endl;
        }
    }
    
    for (auto const &node : fsm.nodes)
    {
        if (node != nullptr)
        {
            out << *node;
        }
    }
    out << endl << "}" << endl;
    return out;
}


unsigned int Fsm::getRandomSeed() {
    
    return static_cast<unsigned int>
    (std::chrono::high_resolution_clock::now().time_since_epoch().count());
    
}


std::unique_ptr<Fsm>
Fsm::createRandomFsm(const string & fsmName,
                     const int maxInput,
                     const int maxOutput,
                     const int maxState,
                     std::unique_ptr<FsmPresentationLayer> &&pl,
                     const unsigned seed) {
    
    // Initialisation of random number generation
    if ( seed == 0 ) {
        srand(getRandomSeed());
    }
    else {
        srand(seed);
    }
    
    // Produce the nodes and put them into a vector.
    // All nodes are marked 'white' by the costructor - this is now
    // used to mark unreachable states which have to be made reachable
    std::vector<std::unique_ptr<FsmNode> > lst;
    for ( int n = 0; n <= maxState; n++ ) {
        lst.emplace_back(new FsmNode(n,fsmName));
    }
    
    // At index 0 of the vector, the initial state is store, and
    // this is reachable
    lst[0]->setColor(FsmNode::black);
    
    // We create transitions by starting from black (reachable) nodes
    // and trying to reach at least one white node from there.
    deque<FsmNode*> bfsq;
    bfsq.push_back(lst[0].get());
    
    while ( not bfsq.empty() ) {
        
        FsmNode *srcNode = bfsq.front();
        bfsq.pop_front();
        
        // Generation part 1.
        // Select an uncovered node at random
        int whiteNodeIndex = rand() % (maxState+1);
        FsmNode *whiteNode = nullptr;
        FsmNode *startNode = lst[whiteNodeIndex].get();
        FsmNode *thisNode = startNode;
        
        do {
            
            if ( thisNode->getColor() == FsmNode::white ) {
                whiteNode = thisNode;
            }
            else {
                whiteNodeIndex = (whiteNodeIndex + 1) % (maxState+1);
                thisNode = lst[whiteNodeIndex].get();
            }
            
        } while ( whiteNode == nullptr and thisNode != startNode );
        
        // Link srcNode by random transition to thisNode
        // and mark thisNode as black. Also insert into BFS queue
        int x0 = -1;
        int y0 = -1;
        
        if ( whiteNode != nullptr ) {
            x0 = rand() % (maxInput+1);
            y0 = rand() % (maxOutput+1);
            std::unique_ptr<FsmTransition> transition { new FsmTransition(srcNode,whiteNode,
                                                            std::unique_ptr<FsmLabel>(new FsmLabel(x0,y0,pl.get()))) };
            // Add transition to adjacency list of the source node
            srcNode->addTransition(std::move(transition));
            thisNode->setColor(FsmNode::black);
            bfsq.push_back(thisNode);
        }
        
        // Generation part 2.
        // Random transition generation.
        // At least one transition for every input, with
        // arbitrary target nodes.
        for ( int x = 0; x <= maxInput; x++ ) {
            // If x equals x0 produced already above,
            // we may skip it at random
            if ( x == x0 and (rand() % 2) ) continue;
            
            // How many transitions do we want for input x?
            // We construct at most 2 of these transitions
            int numTrans = rand() % 2;
            for ( int t = 0; t <= numTrans; t++ ) {
                // Which output do we want?
                int y = rand() % (maxOutput+1);
                // Which target node?
                int tgtNodeId = rand() % (maxState+1);
                auto tgtNode = lst[tgtNodeId].get();
                if ( tgtNode->getColor() == FsmNode::white ) {
                    tgtNode->setColor(FsmNode::black);
                    bfsq.push_back(tgtNode);
                }
                std::unique_ptr<FsmTransition> transition { new FsmTransition(srcNode,tgtNode,
                                                                std::unique_ptr<FsmLabel>(new FsmLabel(x,y,pl.get()))) };
                // Add transition to adjacency list of the source node
                srcNode->addTransition(std::move(transition));
            }
        }
    }
    
    return std::unique_ptr<Fsm>(new Fsm(fsmName,maxInput,maxOutput,std::move(lst),std::move(pl)));
}

std::unique_ptr<Fsm> Fsm::createMutant(const std::string & fsmName,
                                       const size_t numOutputFaults,
                                       const size_t numTransitionFaults) const {
    
    srand(getRandomSeed());
    
    auto newPresentationLayer = presentationLayer->clone();
    // Create new nodes for the mutant.
    std::vector<std::unique_ptr<FsmNode> > lst;
    for ( int n = 0; n <= maxState; n++ ) {
        lst.emplace_back(new FsmNode(n,fsmName));
    }
    
    // Now add transitions that correspond exactly to the transitions in
    // this FSM
    for ( int n = 0; n <= maxState; n++ ) {
        auto theNewFsmNodeSrc = lst[n].get();
        auto theOldFsmNodeSrc = nodes[n].get();
        for ( auto &tr : theOldFsmNodeSrc->getTransitions() ) {
            int tgtId = tr->getTarget()->getId();
            auto newLbl = tr->getLabel()->clone();
            std::unique_ptr<FsmTransition> transition { new FsmTransition(theNewFsmNodeSrc,lst[tgtId].get(),std::move(newLbl)) };
            theNewFsmNodeSrc->addTransition(std::move(transition));
        }
    }
    
    // Now add transition faults to the new machine
    for ( size_t tf = 0; tf < numTransitionFaults; tf++ ) {
        int srcNodeId = rand() % (maxState+1);
        int newTgtNodeId = rand() % (maxState+1);
        int trNo = rand() % lst[srcNodeId]->getTransitions().size();
        auto &tr = lst.at(srcNodeId)->getTransitions().at(trNo);
        if ( tr->getTarget()->getId() == newTgtNodeId ) {
            newTgtNodeId = (newTgtNodeId+1) % (maxState+1);
        }
        lst.at(srcNodeId)->getTransitions().at(trNo)->setTarget(lst.at(newTgtNodeId).get());
    }
    
    // Now add output faults to the new machine
    for (size_t of = 0; of < numOutputFaults; of++ ) {
        int srcNodeId = rand() % (maxState+1);
        int trNo = rand() % lst[srcNodeId]->getTransitions().size();
        auto &tr = lst.at(srcNodeId)->getTransitions().at(trNo);
        int theInput = tr->getLabel()->getInput();
        int newOutVal = rand() % (maxOutput+1);
        int originalNewOutVal = rand() % (maxOutput+1);
        bool newOutValOk;
        
        // We don't want to modify this transition in such a way
        // that another one with the same label and the same
        // source/target nodes already exists.
        do {
            
            newOutValOk = true;
            
            for ( auto &trOther : lst[srcNodeId]->getTransitions() ) {
                if ( tr == trOther ) continue;
                if ( trOther->getTarget()->getId() != tr->getTarget()->getId() )
                    continue;
                if ( trOther->getLabel()->getInput() != theInput ) continue;
                if ( trOther->getLabel()->getOutput() == newOutVal ) {
                    newOutValOk = false;
                }
            }
            
            if ( not newOutValOk ) {
                newOutVal = (newOutVal+1) % (maxOutput+1);
            }
            
        } while ( (not newOutValOk) and (originalNewOutVal != newOutVal) );
        
        if ( newOutValOk ) {
            
            std::unique_ptr<FsmLabel> newLbl { new FsmLabel(tr->getLabel()->getInput(),
                                                     newOutVal,
                                                     newPresentationLayer.get()) };
            
            tr->setLabel(std::move(newLbl));
        }
    }
    
    return std::unique_ptr<Fsm>(new Fsm(fsmName,maxInput,maxOutput,std::move(lst), std::move(newPresentationLayer)));
}



std::vector< std::unordered_set<int> >
Fsm::getEquivalentInputsFromPrimeMachine() {
    
    vector< std::unordered_set<int> > v;
    
    shared_ptr<OFSMTable> ot =
        make_shared<OFSMTable>(nodes,
                               maxInput,
                               maxOutput,
                               presentationLayer.get());
    
    // mark all inputs as non-equivalent
    vector<bool> equivalentToSmallerInput;
    for ( int x = 0; x <= maxInput; x++ )
        equivalentToSmallerInput.push_back(false);
    
    // Check inputs for equivalence
    for ( int x1 = 0; x1 <= maxInput; x1++ ) {
        
        if ( equivalentToSmallerInput[x1] ) continue;
        
        unordered_set<int> classOfX1;
        classOfX1.insert(x1);
        
        for ( int x2 = x1 + 1; x2 <= maxInput; x2++ ) {
            
            bool x2EquivX1 = true;
            
            // To check whether x1 is equivalent to x2,
            // loop over all outputs y and compare OFSM
            // table columns x1/y and x2/y
            for ( int y = 0; y <= maxOutput; y++ ) {
                if ( not ot->compareColumns(x1,y,x2,y) ) {
                    x2EquivX1 = false;
                    break;
                }
            }
            
            if ( x2EquivX1 ) {
                equivalentToSmallerInput[x2] = true;
                classOfX1.insert(x2);
            }
            
        }
        
        v.push_back(classOfX1);

    }
    
    return v;
    
}

std::vector< std::unordered_set<int> > Fsm::getEquivalentInputs() {
    
    if ( minimal != True ) {
        return minimise().getEquivalentInputsFromPrimeMachine();
    }
    else {
        return getEquivalentInputsFromPrimeMachine();
    }
    
}


void Fsm::accept(FsmVisitor& v) {
    
    deque< FsmNode* > bfsq;
    
    resetColor();
    
    v.visit(*this);
    
    bfsq.push_back(nodes[initStateIdx].get());
    
    while ( not bfsq.empty() ) {
        FsmNode *theNode = bfsq.front();
        bfsq.pop_front();
        v.setNew(true);
        theNode->accept(v,bfsq);
    }
}



bool Fsm::removeUnreachableNodes(std::vector<std::unique_ptr<FsmNode>>& unreachableNodes) {
    
    vector<std::unique_ptr<FsmNode>> newNodes;
    FsmVisitor v;
    
    // Mark all reachable nodes as 'visited'
    accept(v);
    
    // When removing nodes from the FSM, the node ids of all remaining nodes
    // have to be adapted, in order to match the index in the list of nodes.
    // This is necessary, because during minimisation with OFSM tables or
    // Pk-tables, the algorithms rely on the range of row numbers being
    // identical to the range of node ids of the reachable nodes.
    int subtractFromId = 0;
    for ( auto &n : nodes ) {
        if ( not n->hasBeenVisited() ) {
            presentationLayer->removeState2String(n->getId() - subtractFromId);
            unreachableNodes.push_back(std::move(n));
            ++subtractFromId;
        }
        else {
            n->setId(n->getId() - subtractFromId);
            newNodes.push_back(std::move(n));
        }
    }
    
    nodes = std::move(newNodes);
    
    return (unreachableNodes.size() > 0);
}



bool Fsm::distinguishable(const FsmNode& s1, const FsmNode& s2) {
    
    if ( ofsmTableLst.empty() ) {
        calcOFSMTables();
    }
    
    shared_ptr<OFSMTable> p = ofsmTableLst.back();
    
    return ( p->getS2C().at(s1.getId()) != p->getS2C().at(s2.getId()) );
    
}




