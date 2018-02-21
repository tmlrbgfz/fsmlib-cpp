/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#ifndef FSM_FSM_SEGMENTEDTRACE_H_
#define FSM_FSM_SEGMENTEDTRACE_H_

#include <iostream>
#include <vector>
#include <deque>

#include "fsm/FsmNode.h"
#include "fsm/Trace.h"

class TraceSegment {
    
private:
    std::vector<int> segment;
    size_t prefix;
    FsmNode *tgtNode;
    
public:
    
    TraceSegment();
    TraceSegment(std::vector<int> const &segment,
                 size_t prefix = std::string::npos,
                 FsmNode *tgtNode = nullptr);
    
    /** Shallow copy */
    TraceSegment(const TraceSegment& other);
    
    void setPrefix(size_t pref);
    
    size_t getPrefix() const { return prefix; }
    
    std::vector<int> const &get() const { return segment; }
    std::vector<int> &get() { return segment; }
    
    std::vector<int> getCopy() const;
    Trace getAsTrace(std::unique_ptr<FsmPresentationLayer> && presentationLayer) const;
    
    size_t size() const;
    
    int at(size_t n) const;
    
    FsmNode *getTgtNode() const { return tgtNode; }
    void setTgtNode(FsmNode *tgtNode) { this->tgtNode = tgtNode; }
    
    friend std::ostream & operator<<(std::ostream & out, const TraceSegment& fsm);
};

class SegmentedTrace
{
private:
    
    std::deque< TraceSegment > segments;
    
public:
    
    SegmentedTrace(std::deque< TraceSegment > const &segments);
    SegmentedTrace(const SegmentedTrace& other);
    
    void add(TraceSegment const &seg);
    
    std::vector<int> getCopy() const;
    
    FsmNode *getTgtNode() const;
    
    size_t size() const { return segments.size(); }

    TraceSegment & back() {
         return segments.back();
    }
    
    TraceSegment & front() {
        return segments.front();
    }

    TraceSegment const & back() const {
         return segments.back();
    }
    
    TraceSegment const & front() const {
        return segments.front();
    }
    
    std::deque< TraceSegment > const & getSegments() const { return segments; }
    
    
    friend std::ostream & operator<<(std::ostream & out, const SegmentedTrace& fsm);
    
    friend bool operator==(SegmentedTrace const & trace1, SegmentedTrace const & trace2);
	 
};
#endif  
