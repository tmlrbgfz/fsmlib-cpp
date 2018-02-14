/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#include "trees/OutputTree.h"
#include <fstream>

using namespace std;

void OutputTree::printChildrenOutput(ostream& out, TreeNode const *top, int &idNode, const int idInput) const
{
	int idNodeBase = idNode;
	for (auto const &edge : top->getChildren())
	{
		out << idNodeBase << " -> " << ++ idNode << "[label = \"" << inputTrace.get().at(idInput) << "/" << edge->getIO() << "\" ];" << endl;
		printChildrenOutput(out, edge->getTarget(), idNode, idInput + 1);
	}
}

OutputTree::OutputTree(std::unique_ptr<TreeNode> &&root,
                       InputTrace const &inputTrace,
                       std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
	: Tree(std::move(root), std::move(presentationLayer)), inputTrace(inputTrace)
{

}

OutputTree::OutputTree(InputTrace const &inputTrace,
                       std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
	: Tree(std::move(presentationLayer)), inputTrace(inputTrace)
{

}

std::unique_ptr<OutputTree> OutputTree::clone() const {
	return std::unique_ptr<OutputTree>(new OutputTree(*this));
}

InputTrace OutputTree::getInputTrace() const
{
    return inputTrace;
}

bool OutputTree::contains(OutputTree const &ot) const {
    
    vector<IOTrace> myOutputs = toIOTrace();
    vector<IOTrace> otherOutputs = ot.toIOTrace();
    
    // Check whether every trace of ot is also contained as a trace in this.
	return std::all_of(otherOutputs.cbegin(), otherOutputs.cend(), [&myOutputs](IOTrace const &otherTrace)->bool{
		return std::find(myOutputs.cbegin(), myOutputs.cend(), otherTrace) != myOutputs.cend();
	});
}

void OutputTree::toDot(ostream& out) const
{
	out << "digraph OutputTree {" << endl;
	out << "\trankdir=TB;" << endl;//Top -> Bottom, to create a vertical graph
	out << "\tnode [shape = circle];" << endl;
	int id = 0;
	printChildrenOutput(out, root.get(), id, 0);
	out << "}";
}

void OutputTree::store(std::ofstream& file)
{
	IOListContainer::IOListBaseType lli = getIOLists().getIOLists();
	for (vector<int> const &lst : lli)
	{
		for (unsigned int i = 0; i < lst.size(); ++ i)
		{
			if (i != 0)
			{
				file << std::string(".");
			}

			file << std::string("(") << inputTrace.get().at(i) << std::string(",") << lst.at(i) << std::string(")");
		}
	}
}

std::vector<IOTrace> OutputTree::toIOTrace() const {
    IOListContainer::IOListBaseType lli = getIOLists().getIOLists();
	std::vector<IOTrace> result;
    for (vector<int> const &lst : lli) {
        OutputTrace otrc(lst,std::shared_ptr<FsmPresentationLayer>(presentationLayer->clone().release()));
        IOTrace iotrc(inputTrace,otrc);
        result.push_back(iotrc);
    }
	return result;
}

ostream& operator<<(ostream& out, OutputTree const &ot)
{
	IOListContainer::IOListBaseType lli = ot.getIOLists().getIOLists();
	for (vector<int> const &lst : lli)
	{
		for (unsigned int i = 0; i < lst.size(); ++ i)
		{
            
            if ( i > 0 ) out << ".";

			out << "(" << ot.presentationLayer->getInId(ot.inputTrace.get().at(i)) << "/" << ot.presentationLayer->getOutId(lst.at(i)) << ")";
		}
		out << endl;
	}
	return out;
}

bool operator==(OutputTree const &outputTree1, OutputTree const &outputTree2)
{
    
    return ( outputTree1.contains(outputTree2) and outputTree2.contains(outputTree1) );
    
#if 0
    // This does not work for output trees generated by non-observable FSMs
    
	/*
     * If the associated input traces differ,
	 * the output trees can never be equal
     */
	if (!(outputTree1.inputTrace == outputTree2.inputTrace))
	{
		return false;
	}
    
    /* Check whether all output traces of outputTree1 are contained in outputTree2 */
    
    

	/*Since outputTree1 and outputTree2 are two output trees over the same
	input trace, the trees have the same height. We only have
	to check whether each corresponding node has the same number
	of children, and whether corresponding edges carry the same labels
	(output values).*/
	return *outputTree1.getRoot() == *outputTree2.getRoot();
#endif
}

bool operator!=(OutputTree const &outputTree1, OutputTree const &outputTree2)
{
    return not (outputTree1 == outputTree2);
}

