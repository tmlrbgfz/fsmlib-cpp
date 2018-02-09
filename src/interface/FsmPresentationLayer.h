
/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#ifndef FSM_INTERFACE_FSMPRESENTATIONLAYER_H_
#define FSM_INTERFACE_FSMPRESENTATIONLAYER_H_

#include <istream>
#include <string>
#include <vector>
#include <boost/optional/optional.hpp>

class FsmPresentationLayer
{
private:
	/**
	 * A vector containing a string for each input
	 */
	std::vector<std::string> in2String;

	/**
	 * A vector containing a string for each output
	 */
	std::vector<std::string> out2String;

	/**
	 * A vector containing a string for each state
	 */
	std::vector<std::string> state2String;
public:
	/**
	 * Create a new presentation layer containing nothing
	 */
	FsmPresentationLayer();
    
    /**
     * Copy constructor
     */
    FsmPresentationLayer(const FsmPresentationLayer& pl);

	/**
	 * Create a new presentation layer
	 * @param in2String A vector containing a string for each input
	 * @param out2String A vector containing a string for each output
	 * @param state2String A vector containing a string for each state
	 */
	FsmPresentationLayer(const std::vector<std::string>& in2String,
                         const std::vector<std::string>& out2String,
                         const std::vector<std::string>& state2String);

	/**
	 * Create a new presentation layer
	 * @param inputs An input stream, in which each line stand for one input
	 * @param outputs An input stream, in which each line stand for one output
	 * @param states An input stream, in which each line stand for one state
	 */
	FsmPresentationLayer(std::istream & inputs,
                         std::istream & outputs,
                         std::istream & states);

    void addState2String(std::string name);
    void removeState2String(const int index);

	/**
	 * Getter for a particular input name
	 * @param id The id of the input
	 * @return The name if this input
	 */
	std::string getInId(const unsigned int id) const;

	/**
	 * Getter for a particular output name
	 * @param id The id of the output
	 * @return The name if this output
	 */
	std::string getOutId(const unsigned int id) const;

	/**
	 * Getter for a particular state name
	 * @param id The id of the state
	 * @param prefix If the prefix is not empty, it will be added
     *               at he beginning of the name before returning it
	 * @return The name of this state
	 */
	std::string getStateId(const unsigned int id, const std::string & prefix) const;
    
    /**
     *  Get a const reference of the in2string vector
     */
    std::vector<std::string> const & getIn2String() const { return in2String; }
    
    /**
     *  Get a const reference of the out2string vector
     */
    std::vector<std::string> const & getOut2String() const { return out2String; }
    
    /**
     *  Get a const reference of the state2string vector
     */
    std::vector<std::string> const & getState2String() const { return state2String; }
    
    /**
     *  Convert input name to input number
     */
    boost::optional<int> in2Num(const std::string& name) const;
    
    /**
     *  Convert output name to output number
     */
    boost::optional<int> out2Num(const std::string& name) const;
    
    /**
     *  Convert state name to state number
     */
    boost::optional<int> state2Num(const std::string& name) const;

	/**
	 * Dump the current inputs into an output stream
	 * @param out The standard output to use
	 */
	void dumpIn(std::ostream & out) const;

	/**
	 * Dump the current outputs into an output stream
	 * @param out The standard output to use
	 */
	void dumpOut(std::ostream & out) const;

	/**
	 * Dump the current states into an output stream
	 * @param out The standard output to use
	 */
	void dumpState(std::ostream & out) const;

	/**
	 * Compare two presentation layer to check if they are the same or not
	 * @param otherPresentationLayer The other presentation layer to be compared
	 */
	bool compare(FsmPresentationLayer const *otherPresentationLayer) const;
};
#endif //FSM_INTERFACE_FSMPRESENTATIONLAYER_H_
