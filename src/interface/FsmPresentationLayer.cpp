/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#include "interface/FsmPresentationLayer.h"

FsmPresentationLayer::FsmPresentationLayer()
{

}

FsmPresentationLayer::FsmPresentationLayer(const FsmPresentationLayer& pl)
{
    
    in2String = pl.in2String;
    out2String = pl.out2String;
    state2String = pl.state2String;
    
}

FsmPresentationLayer::FsmPresentationLayer(const std::vector<std::string>& in2String, const std::vector<std::string>& out2String, const std::vector<std::string>& state2String)
	: in2String(in2String), out2String(out2String), state2String(state2String)
{

}

FsmPresentationLayer::FsmPresentationLayer(std::istream& inputs, std::istream& outputs, std::istream& states)
{
	std::string line;

	while (std::getline(inputs, line))
	{
		in2String.push_back(line);
	}

	while (std::getline(outputs, line))
	{
		out2String.push_back(line);
	}

	while (std::getline(states, line))
	{
		state2String.push_back(line);
	}
}

void FsmPresentationLayer::addState2String(std::string name)
{
    state2String.push_back(name);
}

void FsmPresentationLayer::removeState2String(const int index)
{
    if (index >= 0 && state2String.size() > static_cast<size_t>(index))
    {
        state2String.erase(state2String.begin() + index);
    }
}

std::string FsmPresentationLayer::getInId(const unsigned int id) const
{
	if (id >= in2String.size())
	{
		return std::to_string(id);
	}
	return in2String.at(id);
}

std::string FsmPresentationLayer::getOutId(const unsigned int id) const
{
	if (id >= out2String.size())
	{
		return std::to_string(id);
	}
	return out2String.at(id);
}

std::string FsmPresentationLayer::getStateId(const unsigned int id, const std::string & prefix) const
{
	if (id >= state2String.size())
	{
		if (prefix.empty())
		{
			return std::to_string(id);
		}
		return prefix + std::to_string(id);
	}
	return state2String.at(id);
}

void FsmPresentationLayer::dumpIn(std::ostream & out) const
{
	for (unsigned int i = 0; i < in2String.size(); ++ i)
	{
		if (i != 0)
		{
			out << std::endl;
		}
		out << in2String.at(i);
	}
}

void FsmPresentationLayer::dumpOut(std::ostream & out) const
{
	for (unsigned int i = 0; i < out2String.size(); ++ i)
	{
		if (i != 0)
		{
			out << std::endl;
		}
		out << out2String.at(i);
	}
}

void FsmPresentationLayer::dumpState(std::ostream & out) const
{
	for (unsigned int i = 0; i < state2String.size(); ++ i)
	{
		if (i != 0)
		{
			out << std::endl;
		}
		out << state2String.at(i);
	}
}

bool FsmPresentationLayer::compare(FsmPresentationLayer const *otherPresentationLayer) const {
	if (in2String.size() != otherPresentationLayer->in2String.size())
	{
		return false;
	}

	if (out2String.size() != otherPresentationLayer->out2String.size())
	{
		return false;
	}

	for (unsigned int i = 0; i < in2String.size(); ++ i)
	{
		if (in2String.at(i) != otherPresentationLayer->in2String.at(i))
		{
			return false;
		}
	}

	for (unsigned int i = 0; i < out2String.size(); ++ i)
	{
		if (out2String.at(i) != otherPresentationLayer->out2String.at(i))
		{
			return false;
		}
	}
	return true;
}


boost::optional<int> FsmPresentationLayer::in2Num(const std::string& name) const {
    
    for ( size_t i = 0; i < in2String.size(); i++ ) {
        if ( in2String[i] == name ) { 
			return boost::make_optional<int>(i);
		}
    }
    
    return boost::none;
    
}

boost::optional<int> FsmPresentationLayer::out2Num(const std::string& name) const {
    
    for ( size_t i = 0; i < out2String.size(); i++ ) {
        if ( out2String[i] == name ) {
			return boost::make_optional<int>(i);
		}
    }
    
    return boost::none;
}

boost::optional<int> FsmPresentationLayer::state2Num(const std::string& name) const {
    
    for ( size_t i = 0; i < state2String.size(); i++ ) {
        if ( state2String[i] == name ) {
			return boost::make_optional<int>(i);
		}
    }
    
    return boost::none;
}
