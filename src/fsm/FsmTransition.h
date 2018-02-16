/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#ifndef FSM_FSM_FSMTRANSITION_H_
#define FSM_FSM_FSMTRANSITION_H_

#include <memory>

#include "fsm/FsmLabel.h"
#include "fsm/FsmVisitor.h"

class FsmNode;

class FsmTransition
{
private:
	/**
	 *The node from which the transition comes
	 */
	FsmNode *source;

	/**
	 * The node where the transition go
	 */
	FsmNode *target;

	/**
	 * The label of this transition
	 */
	std::unique_ptr<FsmLabel> label;
    
    /**
     *  List of requirements satisfied by the transition
     */
    std::vector<std::string> satisfies;
    
public:
	/**
	Create a FsmTransition
	@param source The node from which the transition come
	@param target The node where the transition go
	@param label The label of this transition
	*/
	FsmTransition(FsmNode *source,
                  FsmNode *target,
                  std::unique_ptr<FsmLabel> &&label);

	/**
	 * Getter for the source
	 * @return The source node of the transition
	 */
	FsmNode * getSource() const;

	/**
	Getter for the target
	@return The target node of tghis transition
	*/
	FsmNode * getTarget() const;
    
    /** setter functions */
    void setSource(FsmNode * src);
    void setTarget(FsmNode * tgt);
    void setLabel(std::unique_ptr<FsmLabel> &&lbl);

    
    /** 
     *   Accept an FsmVisitor and call accept method of the label.
     *   Then call accept method of the target node
     */
    void accept(FsmVisitor& v);
    
	/**
	Getter for the label
	@return The label of this transition
	*/
    FsmLabel * getLabel() const;
    
    /**
     *  Get list of requirements satisified by the transition
     */
    std::vector<std::string>& getSatisfied() { return satisfies; }
    void addSatisfies(std::string req) { satisfies.push_back(req); }

	/**
	Output the FsmTransition to a standard output stream
	@param out The standard output stream to use
	@param transition The FsmTransition to print
	@return The standard output stream used, to allow user to cascade <<
	*/
	friend std::ostream & operator<<(std::ostream & out, FsmTransition & transition);
};
#endif //FSM_FSM_FSMTRANSITION_H_
