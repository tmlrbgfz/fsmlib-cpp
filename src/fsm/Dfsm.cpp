/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 *
 * Licensed under the EUPL V.1.1
 */
#include "fsm/Dfsm.h"
#include "fsm/FsmNode.h"
#include "fsm/FsmTransition.h"
#include "fsm/PkTable.h"
#include "fsm/DFSMTableRow.h"
#include "fsm/InputTrace.h"
#include "fsm/IOTrace.h"
#include "trees/Tree.h"

using namespace std;



shared_ptr<FsmPresentationLayer> Dfsm::createPresentationLayerFromCsvFormat(const string & fname) {
    
    // key comparator for usage in string sets
    class setCmp
    {
    public:
        bool operator()(const string& s1, const string& s2)
        {
            return (s1.compare(s2) < 0);
        }
    };
    // Set containing all output events defined so far
    set<string,setCmp> outStrSet;
    
    
    vector<string> in2String;
    vector<string> out2String;
    vector<string> state2String;
    
    ifstream inputFile(fname);
    
    if ( not inputFile.is_open() ) {
        cout << "Unable to open input file" << endl;
        exit(EXIT_FAILURE);
    }
    
    // Insert a "no operation" output as first member of the
    // set to be filled with the output alphabet members
    outStrSet.insert(string("_nop"));
    
    string line;
    
    // ------------------------------------------------------------
    // Read input names from first line
    // ------------------------------------------------------------
    getline(inputFile,line);
    
    // Skip first field
    size_t pos = line.find(";");
    if ( pos == string::npos ) return nullptr;
    pos++;
    size_t posEnd;
    do {
        
        posEnd = line.find(";", pos);
        
        string newInput;
        if ( posEnd == string::npos ) {
            newInput = line.substr(pos);
        }
        else {
            newInput = line.substr(pos, posEnd - pos);
        }
        
        // Trim
        newInput.erase(0,newInput.find_first_not_of(" \n\r\t\""));
        newInput.erase(newInput.find_last_not_of(" \n\r\t\"")+1);
        
        // Add to vector of strings denoting input names
        in2String.push_back(newInput);
        
        pos = posEnd + 1;
        
    } while ( posEnd != string::npos );
    
    // ------------------------------------------------------------
    // Read state names and outputs from consecutive lines
    // ------------------------------------------------------------
    while (getline(inputFile, line))
    {
        // Get the state name
        posEnd = line.find(";");
        if ( posEnd == string::npos ) continue;
        string newState = line.substr(0,posEnd);
        newState.erase(0,newState.find_first_not_of(" \n\r\t\""));
        newState.erase(newState.find_last_not_of(" \n\r\t\"")+1);
        state2String.push_back(newState);
        
        // Look for new output names
        do {
            
            pos = posEnd + 1;
            posEnd = line.find(";",pos);
            
            string newEntry;
            if ( posEnd == string::npos ) {
                newEntry = line.substr(pos);
            }
            else {
                newEntry = line.substr(pos, posEnd - pos);
            }
            size_t outPos = newEntry.find("/");
            if ( outPos != string::npos ) {
                string outStr = newEntry.substr(outPos+1);
                outStr.erase(0,outStr.find_first_not_of(" \n\r\t\""));
                outStr.erase(outStr.find_last_not_of(" \n\r\t\"")+1);
                outStrSet.insert(outStr);
            }
            
        } while ( posEnd != string::npos );
        
    }
    inputFile.close();
    
    for (const auto &s : outStrSet ) {
        out2String.push_back(s);
    }
    
    maxInput = (int)in2String.size() - 1;
    maxOutput = (int)out2String.size() - 1;
    maxState = (int)state2String.size() - 1;
    initStateIdx = 0;
    
    return make_shared<FsmPresentationLayer>(in2String,out2String,state2String);
    
}



shared_ptr<FsmPresentationLayer>
Dfsm::createPresentationLayerFromCsvFormat(const string & fname,
                                           FsmPresentationLayer const *pl) {
    
    // key comparator for usage in string sets
    class setCmp
    {
    public:
        bool operator()(const string& s1, const string& s2)
        {
            return (s1.compare(s2) < 0);
        }
    };
    // Set containing all output events defined so far
    set<string,setCmp> outStrSet;
    
    
    vector<string> in2String(pl->getIn2String());
    vector<string> out2String(pl->getOut2String());
    vector<string> state2String;
    
    ifstream inputFile(fname);
    
    if ( not inputFile.is_open() ) {
        cout << "Unable to open input file" << endl;
        exit(EXIT_FAILURE);
    }
    
    // Insert a "no operation" output as first member of the
    // output alphabet, if it's not already contained in pl
    if ( pl->out2Num("_nop") < 0 ) {
        outStrSet.insert(string("_nop"));
    }
    
    string line;
    
    // ------------------------------------------------------------
    // Read input names from first line
    // ------------------------------------------------------------
    getline(inputFile,line);
    
    // Skip first field
    size_t pos = line.find(";");
    if ( pos == string::npos ) return nullptr;
    pos++;
    size_t posEnd;
    do {
        
        posEnd = line.find(";", pos);
        
        string newInput;
        if ( posEnd == string::npos ) {
            newInput = line.substr(pos);
        }
        else {
            newInput = line.substr(pos, posEnd - pos);
        }
        
        // Trim
        newInput.erase(0,newInput.find_first_not_of(" \n\r\t\""));
        newInput.erase(newInput.find_last_not_of(" \n\r\t\"")+1);
        
        // Add to vector of strings denoting input names,
        // if this string is not already containe in pl
        if ( pl->in2Num(newInput) < 0 ) {
            in2String.push_back(newInput);
        }
        
        pos = posEnd + 1;
        
    } while ( posEnd != string::npos );
    
    // ------------------------------------------------------------
    // Read state names and outputs from consecutive lines
    // ------------------------------------------------------------
    while (getline(inputFile, line))
    {
        // Get the state name
        posEnd = line.find(";");
        if ( posEnd == string::npos ) continue;
        string newState = line.substr(0,posEnd);
        newState.erase(0,newState.find_first_not_of(" \n\r\t\""));
        newState.erase(newState.find_last_not_of(" \n\r\t\"")+1);
        state2String.push_back(newState);
        
        // Look for new output names
        do {
            
            pos = posEnd + 1;
            posEnd = line.find(";",pos);
            
            string newEntry;
            if ( posEnd == string::npos ) {
                newEntry = line.substr(pos);
            }
            else {
                newEntry = line.substr(pos, posEnd - pos);
            }
            size_t outPos = newEntry.find("/");
            if ( outPos != string::npos ) {
                string outStr = newEntry.substr(outPos+1);
                outStr.erase(0,outStr.find_first_not_of(" \n\r\t\""));
                outStr.erase(outStr.find_last_not_of(" \n\r\t\"")+1);
                
                // Insert to set only if not already contained in pl
                if ( pl->out2Num(outStr) < 0 ) {
                    outStrSet.insert(outStr);
                }
            }
            
        } while ( posEnd != string::npos );
        
    }
    inputFile.close();
    
    for (const auto &s : outStrSet ) {
        out2String.push_back(s);
    }
    
    maxInput = (int)in2String.size() - 1;
    maxOutput = (int)out2String.size() - 1;
    maxState = (int)state2String.size() - 1;
    initStateIdx = 0;
    
    return make_shared<FsmPresentationLayer>(in2String,out2String,state2String);
    
}



void Dfsm::createDfsmTransitionGraph(const string& fname) {
    
    ifstream inputFile(fname);
    size_t pos;
    size_t posEnd;
    string line;
    FsmNode *tgtNode;
    
    // skip first line
    getline(inputFile,line);
    
    // Initialise nodes-vector with null pointers, so that states
    // not yet defined can be identified.
    for ( size_t n = 0; n <= (size_t)maxState; n++ ) nodes.push_back(nullptr);
    
    
    int nodeId = 0;
    while (getline(inputFile, line)) {
        
        currentParsedNode = nodes[nodeId].get();
        if ( currentParsedNode == nullptr ) {
            currentParsedNode = new FsmNode(nodeId,
                                            presentationLayer->getStateId(nodeId,""),
                                            presentationLayer.get());
            nodes[nodeId].reset(currentParsedNode);
        }
        
        // Skip the first column
        pos = line.find(";");
        if ( pos++ == string::npos ) continue;
        int x = 0;
        // Create transitions emanating from currentParsedNode
        do {
            
            string tableEntry;
            
            posEnd = line.find(";",pos);
            if ( posEnd == string::npos ) {
                tableEntry = line.substr(pos);
            }
            else {
                tableEntry = line.substr(pos,posEnd-pos);
            }
            
            // Empty table entries lead to an x/_nop self loop
            // _nop has integer code 0
            tableEntry.erase(0,tableEntry.find_first_not_of(" \n\r\t\""));
            tableEntry.erase(tableEntry.find_last_not_of(" \n\r\t\"")+1);
            if ( tableEntry.empty() ) {
                tgtNode = currentParsedNode;
                std::unique_ptr<FsmLabel> lbl { new FsmLabel(x,0,presentationLayer.get()) };
                std::unique_ptr<FsmTransition> tr { new FsmTransition(currentParsedNode,tgtNode,std::move(lbl)) };
                currentParsedNode->addTransition(std::move(tr));
            }
            else {
                
                // get the target state from the table entry
                size_t i0;
                string tgtStateName;
                i0 = tableEntry.find("/");
                if ( i0 == string::npos ) {
                    tgtStateName = tableEntry;
                }
                else {
                    tgtStateName = tableEntry.substr(0,i0);
                }
                tgtStateName.erase(0,tgtStateName.find_first_not_of(" \n\r\t\""));
                tgtStateName.erase(tgtStateName.find_last_not_of(" \n\r\t\"")+1);
                
                auto tgtStateId = presentationLayer->state2Num(tgtStateName);
                if ( static_cast<bool>(tgtStateId) ) {
                    tgtNode = nodes[tgtStateId.value()].get();
                    if ( tgtNode == nullptr ) {
                        tgtNode = new FsmNode(tgtStateId.value(),
                                              tgtStateName,
                                              presentationLayer.get());
                        nodes[tgtStateId.value()].reset(tgtNode);
                    }
                    // Get the output associated with the current state
                    // and input
                    string outStr =
                    ( i0 == string::npos ) ? "" : tableEntry.substr(i0+1);
                    outStr.erase(0,outStr.find_first_not_of(" \n\r\t\""));
                    outStr.erase(outStr.find_last_not_of(" \n\r\t\"")+1);
                    
                    auto y =
                    ( outStr.empty() )
                    ? boost::none
                    : presentationLayer->out2Num(outStr);
                    
                    if ( static_cast<bool>(y) ) {
                        std::unique_ptr<FsmLabel> lbl { new FsmLabel(x,y.value(),presentationLayer.get()) };
                        std::unique_ptr<FsmTransition> tr { new FsmTransition(currentParsedNode,tgtNode,std::move(lbl)) };
                        currentParsedNode->addTransition(std::move(tr));
                    }
                    
                }
                else {
                    cout << endl << "ERROR: undefined target state "
                    << tgtStateName << endl;
                }
            }
            
            x++;
            pos = posEnd + 1;
            
        } while ( posEnd != string::npos );
        
        
        
        nodeId++;
        
    }
    
    
    inputFile.close();
    
}

void Dfsm::initDistTraces() {
    distTraces.clear();
    vector< vector< vector<int> > > oneRow;
    oneRow.resize(size());
    distTraces.resize(size(), oneRow);
}

Dfsm::Dfsm(const string & fname,
           const string & fsmName) : Fsm(nullptr)   {
    dfsmTable = nullptr;
    name = fsmName;
    presentationLayer = createPresentationLayerFromCsvFormat(fname)->clone();
    createDfsmTransitionGraph(fname);
}

Dfsm::Dfsm(const string & fname,
           const string & fsmName,
           std::unique_ptr<FsmPresentationLayer> &&pl) : Fsm(nullptr)   {
    
    dfsmTable = nullptr;
    name = fsmName;
    presentationLayer = createPresentationLayerFromCsvFormat(fname,pl.get())->clone();
    createDfsmTransitionGraph(fname);
}

void Dfsm::createAtRandom()
{
    srand(getRandomSeed());
    
    for (unsigned int i = 0; i < nodes.size(); ++ i)
    {
        nodes [i].reset(new FsmNode(i, presentationLayer.get()));
    }
    
    for (unsigned int i = 0; i < nodes.size(); ++ i)
    {
        FsmNode *source = nodes.at(i).get();
        
        for (int input = 0; input <= maxInput; ++ input)
        {
            int nTarget = rand() % nodes.size();
            FsmNode *target = nodes.at(nTarget).get();
            int output = rand() % (maxOutput + 1);
            std::unique_ptr<FsmTransition> transition { 
                    new FsmTransition(source,
                                      target,
                                      std::unique_ptr<FsmLabel>(new FsmLabel(input,
                                                                             output,
                                                                             presentationLayer.get()))) };
            source->addTransition(std::move(transition));
        }
    }
}

vector<shared_ptr<PkTable> > Dfsm::getPktblLst() const
{
    return pktblLst;
}

shared_ptr<DFSMTable> Dfsm::toDFSMTable() const
{
    shared_ptr<DFSMTable> tbl
            = make_shared<DFSMTable>(nodes.size(), maxInput, presentationLayer.get());
    
    for (unsigned int i = 0; i < nodes.size(); ++ i)
    {
        if (nodes.at(i) == nullptr)
        {
            continue;
        }
        
        shared_ptr<DFSMTableRow> r = nodes.at(i)->getDFSMTableRow(maxInput);
        
        if (r == nullptr)
        {
            return nullptr;
        }
        tbl->setRow(i, r);
        
        
        
    }
    
    return tbl;
}

Dfsm::Dfsm(const string & fname, const string & fsmName, const int maxNodes, const int maxInput, const int maxOutput, std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
: Fsm(fname, fsmName, maxNodes, maxInput, maxOutput, std::move(presentationLayer))
{
    dfsmTable = nullptr;
}

Dfsm::Dfsm(const string& fname,
           std::unique_ptr<FsmPresentationLayer> &&presentationLayer,
           const string & fsmName)
: Fsm(fname,std::move(presentationLayer),fsmName)
{
    dfsmTable = nullptr;
}

Dfsm::Dfsm(const string & fsmName, const int maxNodes, const int maxInput, const int maxOutput, std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
: Fsm(std::move(presentationLayer))
{
    dfsmTable = nullptr;
    name = fsmName;
    nodes.reserve(maxNodes);
    for(int i = 0; i < maxNodes; ++i) {
        nodes.emplace_back(nullptr);
    }
    initStateIdx = 0;
    this->maxInput = maxInput;
    this->maxOutput = maxOutput;
    currentParsedNode = nullptr;
    createAtRandom();
    ofstream out(getName() + ".txt");
    dumpFsm(out);
    out.close();
}

Dfsm::Dfsm(const string & fsmName,
           const int maxInput,
           const int maxOutput,
           std::vector<std::unique_ptr<FsmNode>> &&lst,
           std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
: Fsm(fsmName, maxInput, maxOutput, std::move(lst), std::move(presentationLayer))
{
    dfsmTable = nullptr;
}

Dfsm::Dfsm(const Fsm & fsm)
: Fsm (fsm)
{
    dfsmTable = nullptr;
    initStateIdx = fsm.getInitStateIdx();;
    minimal = isMinimal();
}



Dfsm::Dfsm(const Json::Value& fsmExport) :
Fsm()
{
    
    dfsmTable = nullptr;
    if (!fsmExport.isObject()) {
        cerr << endl << "File format is JSON but NOT FSM-lib file structure.";
        return;
    }
    
    bool valid = true;
    Json::Value inputs = fsmExport["inputs"];
    Json::Value outputs = fsmExport["outputs"];
    Json::Value states = fsmExport["states"];
    Json::Value transitions = fsmExport["transitions"];
    Json::Value requirements = fsmExport["requirements"];
    
    map< string,FsmNode* > name2node;
    
    // check JSON value for a valid FSM export
    if (inputs.isNull() || (!inputs.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of FSM inputs from JSON export file structure.";
    }
    if (outputs.isNull() || (!outputs.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of FSM outputs from JSON export file structure.";
    }
    if (states.isNull() || (!states.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of FSM states from JSON export file structure.";
    }
    if (transitions.isNull() || (!transitions.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of FSM transitions from JSON export file structure.";
    }
    if (requirements.isNull() || (!requirements.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of requirements from JSON export file structure.";
    }
    if (!valid) {
        return;
    }
    
    // iterate over all inputs
    vector<string> in2String;
    for (const auto &input : inputs) {
        in2String.push_back(input.asString());
    }
    
    // iterate over all outputs
    vector<string> out2String;
    int theNopNo;
    bool haveNop = false;
    for (unsigned int index = 0; index < outputs.size(); ++index ) {
        string outStr(outputs[index].asString());
        if ( outStr == "_nop" ) {
            haveNop = true;
            theNopNo = index;
        }
        out2String.push_back(outStr);
    }
    
    // Add a NOP output for the case where the FSM is incomplete
    if ( not haveNop ) {
        out2String.push_back("_nop");
        theNopNo = (int)out2String.size() - 1;
    }
    
    // iterate over all states, insert initial state at index 0
    // of the state2String vector.
    vector<string> state2String;
    for (const auto &state : states) {
        if (state["initial"].asBool()) {
            state2String.push_back(state["name"].asString());
            break;
        }
    }
    for (const auto &state : states) {
        if (state["initial"].asBool()) {
            continue; // Initial state has already been inserted
        }
        state2String.push_back(state["name"].asString());
    }
    
    // Create the presentation layer
    presentationLayer.reset(new FsmPresentationLayer(in2String,out2String,state2String));
    
    // Define basic attributes
    name = "FSM";
    currentParsedNode = nullptr;
    maxInput = (int)in2String.size() - 1;
    maxOutput = (int)out2String.size() - 1;
    maxState = (int)state2String.size() - 1;
    initStateIdx = 0;
    minimal = Maybe;
    
    
    // Create all FSM states
    for ( size_t s = 0; s < state2String.size(); s++ ) {
        nodes.emplace_back(new FsmNode((int)s,state2String[s],presentationLayer.get()));
        name2node[state2String[s]] = nodes.back().get();
    }
    
    
    // Create all transitions
    for (const auto &transition : transitions) {
        // Handle source and target nodes
        string srcName(transition["source"].asString());
        string tgtName(transition["target"].asString());
        
        FsmNode *srcNode = name2node[srcName];
        FsmNode *tgtNode = name2node[tgtName];
        
        if ( srcNode == nullptr ) {
            cerr << "Cannot associated valid FSM node with source node name"
            << srcName << endl;
            exit(1);
        }
        
        if ( tgtNode == nullptr ) {
            cerr << "Cannot associated valid FSM node with target node name"
            << tgtName << endl;
            exit(1);
        }
        
        // Get the output
        string yString(transition["output"].asString());
        
        // Trim
        yString.erase(0,yString.find_first_not_of(" \n\r\t\""));
        yString.erase(yString.find_last_not_of(" \n\r\t\"")+1);
        
        auto y = presentationLayer->out2Num(yString);
        
        if ( !y ) {
            cerr << "Unidentified output symbol `"
            <<  yString
            << "' in transition "
            << srcName << " --> " << tgtName
            << endl;
            exit(1);
        }
        
        // For each input, create a separate transition
        // and add it to the source node
        Json::Value inputlist = transition["input"];
        for (const auto &inidx : inputlist) {
            
            string xString(inidx.asString());
            // Trim
            xString.erase(0,xString.find_first_not_of(" \n\r\t\""));
            xString.erase(xString.find_last_not_of(" \n\r\t\"")+1);
            auto x = presentationLayer->in2Num(xString);
            if ( !x ) {
                cerr << "Unidentified input symbol `"
                <<  xString
                << "' in transition "
                << srcName << " --> " << tgtName
                << endl;
                exit(1);
            }
            std::unique_ptr<FsmLabel> theLabel { new FsmLabel(x.value(),y.value(),presentationLayer.get()) };
            std::unique_ptr<FsmTransition> tr { new FsmTransition(srcNode,tgtNode,std::move(theLabel)) };
            
            // Record the requirements satisfied by the transition
            Json::Value satisfies = transition["requirements"];
            for (const auto &satisfie : satisfies) {
                tr->addSatisfies(satisfie.asString());
            }
            
            srcNode->addTransition(std::move(tr));
        }
        
    }
    
    // Add requirements to nodes
    for (const auto &state : states) {
        string nodeName(state["name"].asString());
        
        for (const auto &n : nodes ) {
            if ( n->getName() == nodeName ) {
                Json::Value satisfies = state["requirements"];
                for (const auto &satisfie : satisfies) {
                    n->addSatisfies(satisfie.asString());
                }
            }
        }
        
    }
    
    
    // Force DFSM to be completely defined:
    // For each node and each input event x that has not been used
    // in any outgoing transition of this node,
    // create a self-loop transition with label x/NOP
    for ( auto &n : nodes ) {
        
        vector<bool> inputs(maxInput+1, false);
        for (const auto &tr : n->getTransitions() ) {
            inputs[tr->getLabel()->getInput()] = true;
        }
        for ( int x = 0; x <= maxInput; x++ ) {
            if ( not inputs[x] ) {
                std::unique_ptr<FsmLabel> theLabel { new FsmLabel(x,theNopNo,presentationLayer.get()) };
                std::unique_ptr<FsmTransition> tr { new FsmTransition(n.get(),n.get(),std::move(theLabel)) };
                n->addTransition(std::move(tr));
            }
        }
        
    }
    
}



Dfsm::Dfsm(const Json::Value& fsmExport,
           FsmPresentationLayer const *pl) :
Fsm()
{
    
    dfsmTable = nullptr;
    if (!fsmExport.isObject()) {
        cerr << endl << "File format is JSON but NOT FSM-lib file structure.";
        return;
    }
    
    if ( pl == nullptr ) {
        cerr << endl << "Undefined presentation layer.";
        return;
    }
    
    bool valid = true;
    Json::Value inputs = fsmExport["inputs"];
    Json::Value outputs = fsmExport["outputs"];
    Json::Value states = fsmExport["states"];
    Json::Value transitions = fsmExport["transitions"];
    Json::Value requirements = fsmExport["requirements"];
    
    map< string,FsmNode* > name2node;
    
    // check JSON value for a valid FSM export
    if (inputs.isNull() || (!inputs.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of FSM inputs from JSON export file structure.";
    }
    if (outputs.isNull() || (!outputs.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of FSM outputs from JSON export file structure.";
    }
    if (states.isNull() || (!states.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of FSM states from JSON export file structure.";
    }
    if (transitions.isNull() || (!transitions.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of FSM transitions from JSON export file structure.";
    }
    if (requirements.isNull() || (!requirements.isArray())) {
        valid = false;
        cout << endl << "Unable to extract expected array of requirements from JSON export file structure.";
    }
    if (!valid) {
        return;
    }
    
    
    
    // iterate over all inputs; add all inputs not already contained
    // in pl to in2String.
    vector<string> in2String(pl->getIn2String());
    for (const auto &input : inputs) {
        string theInput(input.asString());
        if ( pl->in2Num(theInput) < 0 ) {
            in2String.push_back(theInput);
        }
    }
    
    // iterate over all outputs
    vector<string> out2String(pl->getOut2String());
    for (const auto &output : outputs) {
        string theOutput(output.asString());
        if ( pl->out2Num(theOutput) < 0 ) {
            out2String.push_back(theOutput);
        }
    }
    // Check whether the _nop output is already contained in pl,
    // otherwise add it to out2String
    auto theNopNo = pl->out2Num("_nop");
    if ( !theNopNo ) {
        out2String.push_back("_nop");
    }
    
    
    // iterate over all states, insert initial state at index 0
    // of the state2String vector.
    vector<string> state2String;
    for (const auto &state : states) {
        if (state["initial"].asBool()) {
            state2String.push_back(state["name"].asString());
            break;
        }
    }
    for (const auto &state : states) {
        if (state["initial"].asBool()) {
            continue; // Initial state has already been inserted
        }
        state2String.push_back(state["name"].asString());
    }
    
    // Create the presentation layer
    presentationLayer.reset(new FsmPresentationLayer(in2String,out2String,state2String));
    
    // Define basic attributes
    name = "FSM";
    currentParsedNode = nullptr;
    maxInput = (int)in2String.size() - 1;
    maxOutput = (int)out2String.size() - 1;
    maxState = (int)state2String.size() - 1;
    initStateIdx = 0;
    minimal = Maybe;
    
    
    // Create all FSM states
    for ( size_t s = 0; s < state2String.size(); s++ ) {
        nodes.emplace_back(new FsmNode((int)s,state2String[s],presentationLayer.get()));
        name2node[state2String[s]] = nodes.back().get();
    }
    
    
    // Create all transitions
    for (const auto &transition : transitions) {
        // Handle source and target nodes
        string srcName(transition["source"].asString());
        string tgtName(transition["target"].asString());
        
        FsmNode *srcNode = name2node[srcName];
        FsmNode *tgtNode = name2node[tgtName];
        
        if ( srcNode == nullptr ) {
            cerr << "Cannot associated valid FSM node with source node name"
            << srcName << endl;
            exit(1);
        }
        
        if ( tgtNode == nullptr ) {
            cerr << "Cannot associated valid FSM node with target node name"
            << tgtName << endl;
            exit(1);
        }
        
        // Get the output
        string yString(transition["output"].asString());
        
        // Trim
        yString.erase(0,yString.find_first_not_of(" \n\r\t\""));
        yString.erase(yString.find_last_not_of(" \n\r\t\"")+1);
        
        auto y = presentationLayer->out2Num(yString);
        
        if ( !y ) {
            cerr << "Unidentified output symbol `"
            <<  yString
            << "' in transition "
            << srcName << " --> " << tgtName
            << endl;
            exit(1);
        }
        
        // For each input, create a separate transition
        // and add it to the source node
        Json::Value inputlist = transition["input"];
        for (const auto &inidx : inputlist) {
            
            string xString(inidx.asString());
            // Trim
            xString.erase(0,xString.find_first_not_of(" \n\r\t\""));
            xString.erase(xString.find_last_not_of(" \n\r\t\"")+1);
            auto x = presentationLayer->in2Num(xString);
            if ( !x ) {
                cerr << "Unidentified input symbol `"
                <<  xString
                << "' in transition "
                << srcName << " --> " << tgtName
                << endl;
                exit(1);
            }
            std::unique_ptr<FsmLabel> theLabel { new FsmLabel(x.value(),y.value(),presentationLayer.get()) };
            std::unique_ptr<FsmTransition> tr { new FsmTransition(srcNode,tgtNode,std::move(theLabel)) };
            
            // Record the requirements satisfied by the transition
            Json::Value satisfies = transition["requirements"];
            for (const auto &satisfie : satisfies) {
                tr->addSatisfies(satisfie.asString());
            }
            
            srcNode->addTransition(std::move(tr));
        }
        
    }
    
    // Add requirements to nodes
    for (const auto &state : states) {
        string nodeName(state["name"].asString());
        
        for (const auto &n : nodes ) {
            if ( n->getName() == nodeName ) {
                Json::Value satisfies = state["requirements"];
                for (const auto &satisfie : satisfies) {
                    n->addSatisfies(satisfie.asString());
                }
            }
        }
        
    }
    
    
    // Force DFSM to be completely defined:
    // For each node and each input event x that has not been used
    // in any outgoing transition of this node,
    // create a self-loop transition with label x/NOP
    for ( auto &n : nodes ) {
        
        vector<bool> inputs(maxInput+1, false);
        for (const auto &tr : n->getTransitions() ) {
            inputs[tr->getLabel()->getInput()] = true;
        }
        for ( int x = 0; x <= maxInput; x++ ) {
            if ( not inputs[x] ) {
                std::unique_ptr<FsmLabel> theLabel { new FsmLabel(x,theNopNo.value(),presentationLayer.get()) };
                std::unique_ptr<FsmTransition> tr { new FsmTransition(n.get(),n.get(),std::move(theLabel)) };
                n->addTransition(std::move(tr));
            }
        }
        
    }
    
}


void Dfsm::calcPkTables() {
    
    dfsmTable = toDFSMTable();
    
    pktblLst.clear();
    shared_ptr<PkTable> p1 = dfsmTable->getP1Table();
    pktblLst.push_back(p1);
    
    for (shared_ptr<PkTable> pk = p1->getPkPlusOneTable();
         pk != nullptr;
         pk = pk->getPkPlusOneTable())
    {
        pktblLst.push_back(pk);
    }
    
#if 0
    cout << "MINIMISE" << endl;
    for (auto p : pktblLst) {
        
        cout << *p << endl;
    }
#endif
    
}


Dfsm Dfsm::minimise()
{
    
    std::vector<std::unique_ptr<FsmNode>> uNodes;
    removeUnreachableNodes(uNodes);
    
    calcPkTables();
    shared_ptr<PkTable> pMin = pktblLst[pktblLst.size()-1];
    
    auto dfsm = pMin->toFsm(name, maxOutput);
    dfsm.calcPkTables();
    return dfsm;
}

void Dfsm::printTables() const
{
    ofstream file("tables.tex");
    if (dfsmTable != nullptr)
    {
        file << *dfsmTable;
    }
    
    for (const auto &i : pktblLst) {
        file << *i << endl << endl;
    }
    file.close();
}

IOListContainer Dfsm::getCharacterisationSet()
{
    /*Create Pk-tables for the minimised FSM*/
    dfsmTable = toDFSMTable();
    pktblLst.clear();
    shared_ptr<PkTable> p1 = dfsmTable->getP1Table();
    pktblLst.push_back(p1);
    
    for (shared_ptr<PkTable> pk = p1->getPkPlusOneTable();
         pk != nullptr;
         pk = pk->getPkPlusOneTable())
    {
        pktblLst.push_back(pk);
    }
    
    
#if 0
    
    cout << "Dfsm::getCharacterisationSet()" << endl;
    cout << *p1 << endl;
    for (auto p : pktblLst) {
        
        cout << *p << endl;
    }
    
#endif
    
    /*Create an empty characterisation set as an empty InputTree instance*/
    characterisationSet.reset(new Tree(presentationLayer->clone()));
    
    /*Loop over all non-equal pairs of states. If they are not already distinguished by
     the input sequences contained in w, create a new input traces that distinguishes them
     and add it to w.*/
    for (unsigned int left = 0; left < nodes.size(); ++ left)
    {
        FsmNode *leftNode = nodes.at(left).get();
        for (unsigned int right = left + 1; right < nodes.size(); ++ right)
        {
            FsmNode *rightNode = nodes.at(right).get();
            
            if (leftNode->distinguished(rightNode, characterisationSet.get()) != nullptr)
            {
                continue;
            }
            
            /*We have to create a new input trace and add it to w, because
             leftNode and rightNode are not distinguished by the current
             input traces contained in w. This step is performed
             according to Gill's algorithm.*/
            InputTrace i = leftNode->calcDistinguishingTrace(rightNode, pktblLst, maxInput);
            IOListContainer::IOListBaseType lli;
            lli.push_back(i.get());
            IOListContainer tcli = IOListContainer(lli, presentationLayer->clone());
            characterisationSet->addToRoot(tcli);
            
#if 0
            cout << "Distinguishing trace for " << nodes[left]->getName()
            << " and " << nodes[right]->getName() << ": " << endl;
            cout << i << endl;
#endif
            
        }
    }
    
#if 0
    /* CHECK */
    for (unsigned int left = 0; left < nodes.size(); ++ left)
    {
        shared_ptr<FsmNode> leftNode = nodes.at(left);
        for (unsigned int right = left + 1; right < nodes.size(); ++ right)
        {
            shared_ptr<FsmNode> rightNode = nodes.at(right);
            
            if (leftNode->distinguished(rightNode, characterisationSet) == nullptr)
            {
                cerr << "ERROR: nodes " << leftNode->getName() << " and " << rightNode->getName() << " cannot be distinguished by W" << endl;
            }
            
        }
    }
#endif
    
    // Minimise characterisation set
    // minimiseCharSet(characterisationSet);
    
    /* Wrap list of lists by an IOListContainer instance */
    IOListContainer tcl = characterisationSet->getIOLists();
    return tcl;
}

IOTrace Dfsm::applyDet(const InputTrace & i)
{
    OutputTrace o = OutputTrace(presentationLayer->clone());
    
    FsmNode *currentNode = nodes.at(initStateIdx).get();
    
    // Apply input trace to FSM, as far as possible
    for ( int input : i.get() ) {
        if ( currentNode == nullptr ) break;
        currentNode = currentNode->apply(input, o);
    }
    
    // Handle the case where the very first input is not accpeted
    // by the incomplete DFSM, or even the initial node does not exist:
    // we return an empty IOTrace
    if (currentNode == nullptr && o.get().empty())
    {
        return IOTrace(InputTrace(presentationLayer->clone()),
                       OutputTrace(presentationLayer->clone()));
    }
    
    // Handle the case where only a prefix of the input trace
    // has been accepted by the incomplete DFSM: we return
    // an IOTrace whose input consist of this prefix, together
    // with the associated outputs already contained in o.
    if (currentNode == nullptr)
    {
        
        // Constant iterator to start of input trace.
        auto ifirst = i.cbegin();
        // Iterator pointing BEHIND the last input applied
        // @note The number of inputs processed so far equals o.size()
        auto ilast = ifirst + o.get().size();
        
        // Constant iterator to start of output trace.
        auto ofirst = o.cbegin();
        // Iterator pointing BEHIND last obtained output.
        auto olast = ofirst + o.get().size();
        
        return IOTrace(InputTrace(vector<int>(ifirst, ilast), presentationLayer->clone()),
                       OutputTrace(vector<int>(ofirst, olast), presentationLayer->clone()));
        
    }
    
    // The full input trace has been processed by the DFSM.
    // The associated outputs are contained in o.
    return IOTrace(InputTrace(i.get(), presentationLayer->clone()),
                   OutputTrace(o.get(), presentationLayer->clone()));
    
}

bool Dfsm::pass(const IOTrace & io)
{
    IOTrace myIO = applyDet(io.getInputTrace());
    return myIO.getOutputTrace() == io.getOutputTrace();
}



IOListContainer Dfsm::wMethod(const unsigned int numAddStates) {
    
    Dfsm dfsmMin = minimise();
    return dfsmMin.wMethodOnMinimisedDfsm(numAddStates);
    
}


IOListContainer Dfsm::wMethodOnMinimisedDfsm(const unsigned int numAddStates)
{
    
    shared_ptr<Tree> iTree = getTransitionCover();
    
    if (numAddStates > 0)
    {
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

IOListContainer Dfsm::wpMethod(const unsigned int numAddStates)
{
    Dfsm dfsmMin = minimise();
    return dfsmMin.wpMethodOnMinimisedDfsm(numAddStates);
}

IOListContainer Dfsm::wpMethodOnMinimisedDfsm(const unsigned int numAddStates)
{
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

IOListContainer Dfsm::hsiMethod(const unsigned int numAddStates)
{
    Fsm fMin = minimiseObservableFSM();
    return fMin.hsiMethod(numAddStates);
}

IOListContainer Dfsm::tMethod()
{
    
    shared_ptr<Tree> iTree = getTransitionCover();
    
    return iTree->getIOLists();
    
}


void Dfsm::toCsv(const string& fname) {
    ofstream out(fname + ".csv");
    
    // Table heading contains input identifiers
    for ( int x = 0; x <= maxInput; x++ ) {
        out << " ; ";
        out << presentationLayer->getInId(x);
    }
    
    for (const auto &node : nodes) {
        out << endl << "\"" << node->getName() << "\"";
        
        for ( int x = 0; x <= maxInput; x++ ) {
            
            out << " ; ";
            
            for (const auto &tr : node->getTransitions() ) {
                if ( tr->getLabel()->getInput() == x ) {
                    out << "\"" << tr->getTarget()->getName()
                    << " / "
                    << presentationLayer->getOutId(tr->getLabel()->getOutput())
                    << "\"";
                    break;
                }
            }
            
        }
        
    }
    
    out << endl;
    out.close();
}

InputTrace Dfsm::calcDistinguishingTrace(
        InputTrace const *iAlpha,
        InputTrace const *iBeta,
        Tree const *tree)
{
    FsmNode *s0 = getInitialState();
    FsmNode *s1 = *s0->after(*iAlpha).begin();
    FsmNode *s2 = *s0->after(*iBeta).begin();

    InputTrace gamma = calcDistinguishingTraceInTree(s1, s2, tree);
    if (!gamma.get().empty())
        return gamma;

    InputTrace gamma2 = calcDistinguishingTraceAfterTree(s1, s2, tree);
    if (!gamma2.get().empty())
        return gamma2;

    return s1->calcDistinguishingTrace(s2,
                                       pktblLst,
                                       maxInput);
}


vector<int> Dfsm::calcDistinguishingTrace(SegmentedTrace *alpha,
                                          SegmentedTrace *beta, TreeNode const *treeNode) {
    FsmNode *s1 = alpha->getTgtNode();
    FsmNode *s2 = beta->getTgtNode();
    
    std::unique_ptr<Tree> tree { new Tree(treeNode->clone(),presentationLayer->clone()) };
    
    InputTrace gamma = calcDistinguishingTraceInTree(s1, s2, tree.get());
    if (!gamma.get().empty())
        return gamma.get();
    
    InputTrace gamma2 = calcDistinguishingTraceAfterTree(s1, s2, tree.get());
    if (!gamma2.get().empty())
        return gamma2.get();
    
    return s1->calcDistinguishingTrace(s2,
                                       pktblLst,
                                       maxInput).get();
}




InputTrace Dfsm::calcDistinguishingTraceInTree(
        FsmNode const *s_i,
        FsmNode const *s_j,
        Tree const *tree)
{
    TreeNode *root = tree->getRoot();
    TreeNode *currentNode = root;
    deque<std::unique_ptr<InputTrace>> q1;

    /* initialize queue */
    for (auto const &e : currentNode->getChildren())
    {
        std::unique_ptr<InputTrace> itrc { new InputTrace(presentationLayer->clone()) };
        itrc->add(e->getIO());
        q1.push_back(std::move(itrc));
    }

    /* Breadth-first search */
    while(!q1.empty())
    {
        std::unique_ptr<InputTrace> itrc { std::move(q1.front()) };
        q1.pop_front();

        if(s_i->distinguished(s_j, itrc->get()))
        {
            return *(itrc.release());
        }

        currentNode = root->after(itrc->cbegin(), itrc->cend());

        for (auto const &ne : currentNode->getChildren())
        {
            std::unique_ptr<InputTrace> itrcTmp { new InputTrace(itrc->get(), presentationLayer->clone()) };
            vector<int>nItrc;
            nItrc.push_back(ne->getIO());
            itrcTmp->append(nItrc);
            q1.push_back(std::move(itrcTmp));
        }
    }
    // Return empty trace: no distinguishing trace found in tree
    return InputTrace(presentationLayer->clone());
}

InputTrace Dfsm::calcDistinguishingTraceInTree(InputTrace const *alpha, InputTrace const *beta, Tree const *tree)
{
    // Only one element in the set, since FSM is deterministic
    FsmNode *s_i = *(getInitialState()->after(*alpha)).begin();
    FsmNode *s_j = *(getInitialState()->after(*beta)).begin();
    return calcDistinguishingTraceInTree(s_i, s_j, tree);
}

InputTrace Dfsm::calcDistinguishingTraceAfterTree(
        FsmNode *s_i,
        FsmNode *s_j,
        Tree const *tree)
{
    for(TreeNode const *leaf : tree->getLeaves())
    {
        InputTrace itrc(leaf->getPath(), presentationLayer->clone());
        FsmNode *s_i_after_input = *(s_i->after(itrc)).begin();
        FsmNode *s_j_after_input = *(s_j->after(itrc)).begin();
        
        // Cannot find distinguishing trace if the states reached
        // by the path are identical
        if(s_i_after_input == s_j_after_input ) continue;

        // Since we are dealing with a minimised DFSM, a distinguishing
        // trace can ALWAYS be found, if s_i_after_input != s_j_after_input
        InputTrace gamma = s_i_after_input->calcDistinguishingTrace(s_j_after_input,
                                                                    pktblLst,
                                                                    maxInput);

        itrc.append(gamma.get());

        return itrc;
    }
    
    // Return empty trace: could not find a tree extension
    // distinguishing s_i and s_j
    return InputTrace(presentationLayer->clone());
}

IOListContainer Dfsm::hMethodOnMinimisedDfsm(const unsigned int numAddStates) {
    
    // Our initial state
    FsmNode *s0 = getInitialState();
    
    // We need a valid set of DFSM table and Pk-Tables for this method
    if ( dfsmTable == nullptr ) {
        calcPkTables();
    }
    
    // Auxiliary state cover set needed for further computations
    std::unique_ptr<Tree> V = getStateCover();
    
    // Test suite is initialised with the state cover
    std::unique_ptr<Tree> iTree = V->clone();
    
    IOListContainer inputEnum = IOListContainer(maxInput,
                                                (int)numAddStates+1,
                                                (int)numAddStates+1,
                                                presentationLayer->clone());
    
    // Initial test suite set is V.Sigma^{m-n+1}, m-n = numAddStates
    iTree->add(inputEnum);
    
    // Step 1.
    // Add all alpha.gamma, beta.gamma where alpha, beta in V
    // and gamma distinguishes s0-after-alpha, s0-after-beta
    // (if alpha.gamma or beta.gamma are already in iTree, addition
    // will not lead to a new test case)
    IOListContainer iolcV = V->getIOListsWithPrefixes();
    IOListContainer::IOListBaseType iolV = iolcV.getIOLists();
    
    for ( size_t i = 0; i < iolV.size(); i++ ) {
        InputTrace alpha(iolV.at(i),presentationLayer->clone());

        for ( size_t j = i+1; j < iolV.size(); j++ ) {
            
            InputTrace beta(iolV.at(j),presentationLayer->clone());

            std::unique_ptr<Tree> alphaTree = iTree->getSubTree(&alpha);
            std::unique_ptr<Tree> betaTree = iTree->getSubTree(&beta);
            std::unique_ptr<Tree> prefixRelationTree = alphaTree->getPrefixRelationTree(betaTree.get());

            InputTrace gamma = calcDistinguishingTrace(&alpha, &beta, prefixRelationTree.get());

            InputTrace iAlphaGamma(alpha.get(), presentationLayer->clone());
            iAlphaGamma.append(gamma);

            InputTrace iBetaGamma(beta.get(), presentationLayer->clone());
            iBetaGamma.append(gamma);

            iTree->addToRoot(iAlphaGamma.get());
            iTree->addToRoot(iBetaGamma.get());
        }
        
    }
    
    // Step 2.
    // For each sequence α.β, α ∈ Q, |β| = m – n + 1, and each non-empty prefix
    // β1 of β that takes the DFSM from s0 to state s,
    // add sequences α.β1.γ and ω.γ, where ω ∈ V and s0-after-ω ≠ s,
    // and γ is a distinguishing sequence of states s0-after-α.β1
    // and s0-after-ω.
    IOListContainer allBeta = IOListContainer(maxInput,
                                              1,
                                              (int)numAddStates+1,
                                              presentationLayer->clone());
    
    IOListContainer::IOListBaseType iolAllBeta = allBeta.getIOLists();
    
    for (const auto &beta : iolAllBeta ) {
        
        for (const auto &alpha : iolV ) {
            
            InputTrace iAlphaBeta(alpha,presentationLayer->clone());
            iAlphaBeta.append(beta);
            std::unordered_set<FsmNode*> s_alpha_betaSet = s0->after(iAlphaBeta);
            FsmNode *s_alpha_beta = *s_alpha_betaSet.begin();
            
            for ( auto omega : iolV ) {
                InputTrace iOmega(omega,presentationLayer->clone());
                std::unordered_set<FsmNode*> s_omegaSet = s0->after(iOmega);
                FsmNode *s_omega = *s_omegaSet.begin();

                if ( s_alpha_beta == s_omega ) continue;

                std::unique_ptr<Tree> alphaBetaTree = iTree->getSubTree(&iAlphaBeta);
                std::unique_ptr<Tree> trAfterOmega = iTree->getSubTree(&iOmega);
                std::unique_ptr<Tree> prefixRelationTree = alphaBetaTree->getPrefixRelationTree(trAfterOmega.get());

                InputTrace gamma = calcDistinguishingTrace(&iAlphaBeta, &iOmega, prefixRelationTree.get());

                InputTrace iAlphaBetaGamma(iAlphaBeta.get(), presentationLayer->clone());
                iAlphaBetaGamma.append(gamma.get());

                InputTrace iOmegaGamma(iOmega.get(), presentationLayer->clone());
                iOmegaGamma.append(gamma.get());

                iTree->addToRoot(iAlphaBetaGamma.get());
                iTree->addToRoot(iOmegaGamma.get());
            }
            
        }
        
    }
    
    // Step 3.
    // For each sequence α.β,α∈Q,|β|=m–n+1, and each two
    // non-empty prefixes β1 and β2 of β that take the
    // DFSM from state s0-after-alpha
    // to two different states add sequences α.β1.γ and α.β2.γ,
    // where γ is a distinguishing sequence of states
    // s0-after-alpha.beta1 and s0-after-alpha.beta2.
    
    for ( auto alpha : iolV ) {
        
        InputTrace iAlpha(alpha,presentationLayer->clone());
        
        for ( auto beta : inputEnum.getIOLists() ) {
        
            for ( size_t i = 0; i < beta.size() - 1; i++ ) {
                InputTrace iBeta_1(presentationLayer->clone());
                for ( size_t k = 0; k <= i; k++ ) {
                    iBeta_1.add(beta[k]);
                }
                
                for ( size_t j = i+1; j < beta.size(); j++ ) {
                    InputTrace iBeta_2(presentationLayer->clone());
                    for ( size_t k = 0; k <= j; k++ ) {
                        iBeta_2.add(beta[k]);
                    }
                    
                    InputTrace iAlphaBeta_1(alpha,presentationLayer->clone());
                    iAlphaBeta_1.append(iBeta_1.get());
                    
                    InputTrace iAlphaBeta_2(alpha,presentationLayer->clone());
                    iAlphaBeta_2.append(iBeta_2.get());
                    
                    unordered_set<FsmNode*> s1Set = s0->after(iAlphaBeta_1);
                    FsmNode *s1 = *s1Set.begin();
                    
                    unordered_set<FsmNode*> s2Set = s0->after(iAlphaBeta_2);
                    FsmNode *s2 = *s2Set.begin();
                    
                    if ( s1 == s2 ) continue;

                    std::unique_ptr<Tree> afterAlphaBeta1Tree = iTree->getSubTree(&iAlphaBeta_1);
                    std::unique_ptr<Tree> afterAlphaBeta2Tree = iTree->getSubTree(&iAlphaBeta_2);
                    std::unique_ptr<Tree> prefixRelationTree = afterAlphaBeta1Tree->getPrefixRelationTree(afterAlphaBeta2Tree.get());

                    InputTrace gamma = calcDistinguishingTrace(&iAlphaBeta_1, &iAlphaBeta_2, prefixRelationTree.get());

                    InputTrace iAlphaBeta_1Gamma(iAlphaBeta_1.get(), presentationLayer->clone());
                    iAlphaBeta_1Gamma.append(gamma.get());

                    InputTrace iAlphaBeta_2Gamma(iAlphaBeta_2.get(), presentationLayer->clone());
                    iAlphaBeta_2Gamma.append(gamma.get());

                    iTree->addToRoot(iAlphaBeta_1Gamma.get());
                    iTree->addToRoot(iAlphaBeta_2Gamma.get());
                }
            }
        
        }
    }

    return iTree->getIOLists();

}


bool Dfsm::distinguishable(const FsmNode& s1, const FsmNode& s2) {
    
    if ( pktblLst.empty() ) {
        calcPkTables();
    }
    
    shared_ptr<PkTable> p = pktblLst.back();
    
    return ( p->getClass(s1.getId()) != p->getClass(s2.getId()) );
    
}

void Dfsm::calculateDistMatrix() {
    initDistTraces();
    calcPkTables();
    
    for ( size_t n = 0; n < size(); n++ ) {
        for ( size_t m = n+1; m < size(); m++ ) {
            // Skip indistinguishable nodes
            if ( not distinguishable(*nodes[n], *nodes[m]) ) continue;
            
            vector< vector<int> > u = calcDistTraces(*nodes[n],*nodes[m]);
            distTraces[n][m] = u;
            distTraces[m][n] = u;
            
        }
    }
    
    
}

vector< vector<int> >  Dfsm::calcDistTraces(vector<int> const &trc, int id1, int id2) {
    
    vector< vector<int> > v;
    
    for ( int x = 0; x <= maxInput; x++ ) {
        int y1 = dfsmTable->getRow(id1)->getioSection()[x];
        int y2 = dfsmTable->getRow(id2)->getioSection()[x];
        
        if ( y1 != y2 ) {
            vector<int> newTrc(trc);
            newTrc.push_back(x);
            v.push_back(newTrc);
        }
    }
    
    return v;
}

vector< vector<int> > Dfsm::calcDistTraces(size_t l, vector<int> const &trc, int id1, int id2) {
    if ( l == 0 ) return calcDistTraces(trc,id1,id2);
    
    vector< vector<int> > v;
    shared_ptr<PkTable> thisPkTbl = pktblLst[l];
    shared_ptr<PkTable> prevPkTbl = pktblLst[l-1];
    
    for ( int x = 0; x <= maxInput; x++ ) {
        int idNext1 = thisPkTbl->getRow(id1)->getI2PMap()[x];
        int idNext2 = thisPkTbl->getRow(id2)->getI2PMap()[x];

        if ( prevPkTbl->getClass(idNext1) != prevPkTbl->getClass(idNext2) ) {
            vector<int> newTrc(trc);
            newTrc.push_back(x);
            vector< vector<int> > w = calcDistTraces(l-1,newTrc,idNext1,idNext2);
            v.insert(v.end(),w.begin(),w.end());
        }
    }

    return v;
}


vector< vector<int> > Dfsm::calcDistTraces(FsmNode& s1, FsmNode& s2) {
    int id1 = s1.getId();
    int id2 = s2.getId();
    
    // Find first Pk-table distPkTbl,  where s1 and s2 are distinguished
    size_t l = 0;
    shared_ptr<PkTable> distPkTbl = nullptr;
    for ( ; l < pktblLst.size(); l++ ) {
        distPkTbl = pktblLst[l];
        if ( distPkTbl->getClass(id1) != distPkTbl->getClass(id2) )
            break;
    }
    
    return calcDistTraces(l,{},id1,id2);
}


vector< vector<int> > Dfsm::getDistTraces(FsmNode const &s1, FsmNode const &s2) const {
    return distTraces.at(s1.getId()).at(s2.getId());
}
