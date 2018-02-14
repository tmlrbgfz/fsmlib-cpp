/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#include "trees/TreeEdge.h"
#include "trees/TreeNode.h"

TreeEdge::TreeEdge(const int io, std::unique_ptr<TreeNode> &&target)
	: io(io), target(std::move(target))
{

}

TreeEdge::TreeEdge(TreeEdge const &other)
    : io(other.io), target(other.target->clone()) {
}

std::unique_ptr<TreeEdge> TreeEdge::clone() {
    return std::unique_ptr<TreeEdge>( new TreeEdge(*this) );
}

int TreeEdge::getIO() const
{
	return io;
}

TreeNode * TreeEdge::getTarget() const
{
	return target.get();
}
