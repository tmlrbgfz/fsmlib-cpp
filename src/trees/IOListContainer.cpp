/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#include "trees/IOListContainer.h"
#include <numeric>

std::vector<int> IOListContainer::nextLst(const int maxInput, const std::vector<int>& lst) const
{
	std::vector<int> returnValue = lst;
	auto lastNotMax = std::find_if(returnValue.rbegin(), returnValue.rend(), [maxInput](int const &element)->bool{
		return element < maxInput;
	});
	if(lastNotMax == returnValue.rend()) {
		return {};
	}
	std::fill(returnValue.rbegin(), lastNotMax, 0);
	++(*lastNotMax);
	return returnValue;
}

IOListContainer::IOListContainer(IOListBaseType const &iolLst, std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
	: iolLst(iolLst), presentationLayer(std::move(presentationLayer))
{

}

IOListContainer::IOListContainer(const int maxInput, const int minLength, const int maxLenght, std::unique_ptr<FsmPresentationLayer> &&presentationLayer)
	: presentationLayer(std::move(presentationLayer))
{
	for (int len = minLength; len <= maxLenght; ++ len)
	{
		/*Initial list of length len only contains zeroes*/
		std::vector<int> lst;
		for (int j = 0; j < len; ++ j)
		{
			lst.push_back(0);
		}
		iolLst.push_back(lst);

		for (lst = nextLst(maxInput, lst); !lst.empty(); lst = nextLst(maxInput, lst))
		{
			iolLst.push_back(lst);
		}
	}
}

IOListContainer::IOListContainer(std::unique_ptr<FsmPresentationLayer> &&pl)
: presentationLayer(std::move(pl)) {
}

IOListContainer::IOListContainer(IOListContainer const &other)
: iolLst(other.iolLst), presentationLayer(other.presentationLayer->clone()) {
}

IOListContainer::IOListBaseType & IOListContainer::getIOLists() {
	return iolLst;
}

void IOListContainer::add(const Trace & trc)
{
	iolLst.push_back(trc.get());
}

IOListContainer::size_type IOListContainer::size() const
{
	return iolLst.size();
}

std::ostream & operator<<(std::ostream & out, const IOListContainer & ot)
{
	out << "{ ";

	auto concatenateWithSeparator = [](std::string const &value, const std::string &separator, std::string const &concat)->std::string{
		return value + separator + concat;
	};
	auto commaSep = std::bind(concatenateWithSeparator, std::placeholders::_1, ",\n  ", std::placeholders::_2);
	auto dotSep = std::bind(concatenateWithSeparator, std::placeholders::_1, ".", std::placeholders::_2);

	auto translateList = [&ot,&dotSep](IOListContainer::IOListBaseType::value_type const &list)->std::string {
		auto translator = [&ot](IOListContainer::IOListBaseType::value_type::value_type const &value)->std::string {
			if(value == -1) {
				return "eps";
			}
			return ot.presentationLayer->getInId(value);
		};
		std::vector<std::string> translatedVector;
		translatedVector.reserve(list.size());
		std::transform(list.begin(), list.end(), std::back_inserter(translatedVector), translator);
		return std::accumulate(translatedVector.begin() + 1, translatedVector.end(), translatedVector.front(), dotSep);
	};
	std::vector<std::string> translatedLists;
	translatedLists.reserve(ot.iolLst.size());
	std::transform(ot.iolLst.begin(), ot.iolLst.end(), std::back_inserter(translatedLists), translateList);
	out << std::accumulate(translatedLists.begin() + 1, translatedLists.end(), translatedLists.front(), commaSep);
	out << " }";
	return out;
}
