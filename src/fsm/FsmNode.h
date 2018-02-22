/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#ifndef FSM_FSM_FSMNODE_H_
#define FSM_FSM_FSMNODE_H_

#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <deque>

#include "fsm/FsmVisitor.h"
#include "fsm/SegmentedTrace.h"

class FsmTransition;
class Fsm;
class OutputTree;
class Tree;
class TreeNode;
class OutputTrace;
class InputTrace;
class OFSMTable;
class PkTable;
class DFSMTableRow;
class TraceSegment;

class FsmNode
{
private:
    std::vector<std::unique_ptr<FsmTransition>> transitions;
	int id;
	std::string name;
	bool visited;
	int color;
	std::vector<FsmNode*> derivedFrom;
    
    Fsm *fsm;
    
    /**
     *  List of requirements satisfied by the node
     */
    std::vector<std::string> satisfies;
	std::pair<OutputTree, std::unordered_map<TreeNode*, FsmNode*>> apply(const InputTrace& itrc) const;
	void addDerivedFrom(FsmNode *singleNode) {
		derivedFrom.push_back(singleNode);
	}
	template<class ... Args>
	void addDerivedFrom(FsmNode *first, Args ... others) {
		addDerivedFrom(first);
		addDerivedFrom(others...);
	}
    
public:
	const static int white = 0;
	const static int grey = 1;
	const static int black = 2;
	FsmNode(const int id);
	FsmNode(const int id,
            const std::string & name);
	
	FsmNode(FsmNode const &other) = default;
	FsmNode(FsmNode &&other) = default;
    
    /**
     * Add a transition to the node. If another transition with the same label and
     * the same target node already exists, the new transition is silently ignored.
     */
	void addTransition(std::unique_ptr<FsmTransition> &&transition);
    
    
    std::vector<std::unique_ptr<FsmTransition>>& getTransitions();
    std::vector<std::unique_ptr<FsmTransition>> const & getTransitions() const;
    int getId() const;
    void setId(const int id) { this->id = id; }
	std::string getName() const;
	bool hasBeenVisited() const;
	void setVisited();
    void setUnvisited();
	template<class ... Args>
	void setDerivedFrom(FsmNode *first, Args ...args) {
		derivedFrom.clear();
		addDerivedFrom(first, args...);	
	}
	void setDerivedFrom(std::vector<FsmNode*> const &nodes) {
		derivedFrom = nodes;
	}
	//Comparison will fail if the nodes are not exactly in the same order
	bool isDerivedFrom(std::vector<FsmNode*> const &p) const;
	bool isDerivedFrom_subset(std::vector<FsmNode*> const &p) const;
	std::vector<FsmNode*> getDerivedFrom() const;
	FsmNode * apply(const int e, OutputTrace & o) const;
	OutputTree apply(const InputTrace & itrc, bool markAsVisited);

	/**
	Return the set of FsmNode instances reachable from this node after
	having applied the input trace itrc
	@param itrc Input Trace to be applied to the FSM, starting with this FsmNode
	@return Set of FsmNode instances reachable from this node via
	input trace itrc.
	*/
	std::unordered_set<FsmNode*> after(InputTrace const &itrc);
    std::unordered_set<FsmNode*> after(std::vector<int> const &itrc);
    std::unordered_set<FsmNode*> after(TraceSegment const &seg);

	/**
	Return list of nodes that can be reached from this node
	when applying input x

	@param x FSM input, to be applied in this node
	@return empty list, if no transition is defined from this node
	with input x
	list of target nodes reachable from this node under input x
	otherwise.
	*/
	std::vector<FsmNode*> after(const int x) const;
	std::unordered_set<FsmNode*> afterAsSet(const int x) const;
	void setColor(const int color);
	int getColor() const;
	std::shared_ptr<DFSMTableRow> getDFSMTableRow(const int maxInput) const;
	bool distinguished(FsmNode const *otherNode, const std::vector<int>& iLst) const;
	std::unique_ptr<InputTrace> distinguished(FsmNode const *otherNode, Tree const *w) const;

	/**
	Calculate a distinguishing input trace for a DFSM node. The algorithm is based
	on Pk-tables
	@param otherNode The other FSM state, to be distinguished from this FSM state
	@param pktblLst  List of Pk-tables, pre-calculated for this DFSM
	@param maxInput  Maximal value of the input alphabet with range 0..maxInput
	@return Distinguishing trace as instance of InputTrace
	*/
	InputTrace calcDistinguishingTrace(FsmNode *otherNode, const std::vector<std::shared_ptr<PkTable>>& pktblLst, const int maxInput);

	/**
	Calculate a distinguishing input trace for a (potentially nondeterministic)
	FSM node. The algorithm is based on OFSM-tables
	@param otherNode The other FSM state, to be distinguished from this FSM state
	@param ofsmTblLst  List of OFSM-tables, pre-calculated for this FSM
	@param maxInput  Maximal value of the input alphabet with range 0..maxInput
	@param maxOutput Maximal value of the output alphabet in range 0..maxOutput
	@return Distinguishing trace as instance of InputTrace
	*/
	InputTrace calcDistinguishingTrace(FsmNode const *otherNode, const std::vector<std::shared_ptr<OFSMTable>>& ofsmTblLst, const int maxInput, const int maxOutput) const;
	bool isObservable() const;

	/**
	 * Check if outgoing transitions of this node are deterministic
	 */
	bool isDeterministic() const;
    
    /** 
     *  Mark that this noe is the initial node
     */
    void markAsInitial();
    bool isInitial() const;

	void setFsm(Fsm *fsm);
	Fsm *getFsm() const {
		return this->fsm;
	}
    
    /**
     * Accept an FsmVisitor, if this node has not been visited already.
     * If the visitor is accepted, this node calls the accept methods of 
     * all transitions.
     */
    void accept(FsmVisitor& v);
    void accept(FsmVisitor& v,
                std::deque< FsmNode* >& bfsq);
    
    /**
     *  Get list of requirements satisified by the node
     */
    std::vector<std::string>& getSatisfied() { return satisfies; }
    std::vector<std::string> const & getSatisfied() const { return satisfies; }
    void addSatisfies(std::string req) { satisfies.push_back(req); }

    
    
    /** Put node information in dot format into the stream */
	friend std::ostream & operator<<(std::ostream & out, const FsmNode & node);
    
    
	friend bool operator==(FsmNode const & node1, FsmNode const & node2);
};
#endif //FSM_FSM_FSMNODE_H_
