/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#include "fsm/FsmTransition.h"
#include "fsm/FsmNode.h"

using namespace std;

FsmTransition::FsmTransition(FsmNode * source,
                             FsmNode * target,
                             std::unique_ptr<FsmLabel> &&label)
	: source(source), target(target), label(std::move(label))
{
    
    if ( source == nullptr ) {
        cerr << "ERROR: Constructor FsmTransition() called with null pointer as source node" << endl;
    }
    
    if ( target == nullptr ) {
        cerr << "ERROR: Constructor FsmTransition() called with null pointer as target node" << endl;
    }
    
    if ( this->label == nullptr ) {
        cerr << "ERROR: Constructor FsmTransition() called with null pointer as label" << endl;
    }

}

FsmNode * FsmTransition::getSource() const
{
	return source;
}

void FsmTransition::setSource(FsmNode * src) {
    source = src;
}

FsmNode * FsmTransition::getTarget() const
{
	return target;
}

void FsmTransition::setTarget(FsmNode * tgt) {
    target = tgt;
}

void FsmTransition::setLabel(std::unique_ptr<FsmLabel> &&lbl) {
    label = std::move(lbl);
}


FsmLabel * FsmTransition::getLabel() const
{
	return label.get();
}

ostream & operator<<(ostream& out, FsmTransition& transition)
{
	out << transition.getSource()->getId() << " -> " << transition.getTarget()->getId() << "[label=\" " << *transition.label << "   \"];";
	return out;
}

void FsmTransition::accept(FsmVisitor &v) {
    v.visit(*this);
    label->accept(v);
    //target->accept(v);
}
