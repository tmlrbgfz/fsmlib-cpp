/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#include "fsm/Trace.h"

Trace::Trace(std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
	: presentationLayer(std::move(presentationLayer))
{

}

Trace::Trace(const std::vector<int>& trace, std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
	: trace(trace), presentationLayer(std::move(presentationLayer))
{

}

Trace::Trace(Trace const &other)
	: trace(other.trace), presentationLayer(other.presentationLayer->clone()) {
}

void Trace::add(const int e) {
	trace.push_back(e);
}

void Trace::append(const std::vector<int>& traceToAppend) {
	trace.insert(trace.end(), traceToAppend.begin(), traceToAppend.end());
}

std::vector<int> Trace::get() const {
	return trace;
}

std::vector<int>::const_iterator Trace::cbegin() const {
	return trace.cbegin();
}

std::vector<int>::const_iterator Trace::cend() const {
	return trace.cend();
}

bool operator==(Trace const & trace1, Trace const & trace2) {
	return trace1.get() == trace2.get();
}

bool operator==(Trace const & trace1, std::vector<int> const & trace2) {
	return trace1.get() == trace2;
}

std::ostream & operator<<(std::ostream & out, const Trace & trace)
{
	for (auto it = trace.cbegin(); it != trace.cend(); ++ it)
	{
		if (it != trace.cbegin())
		{
			out << ".";
		}
		out << *it;
	}
	return out;
}
