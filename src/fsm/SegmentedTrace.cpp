/*
 * Copyright. Gaël Dottel, Christoph Hilken, and Jan Peleska 2016 - 2021
 * 
 * Licensed under the EUPL V.1.1
 */
#include "fsm/SegmentedTrace.h"

using namespace std;

TraceSegment::TraceSegment() {
    segment = vector<int>();
    prefix = string::npos;
    tgtNode = nullptr;
}

TraceSegment::TraceSegment(std::vector<int> const &segment,
                           size_t prefix,
                           FsmNode *tgtNode)
{
    this->segment = segment;
    this->prefix = prefix;
    this->tgtNode = tgtNode;
}

TraceSegment::TraceSegment(TraceSegment const &other) {
    segment = other.segment;
    prefix = other.prefix;
    tgtNode = other.tgtNode;
}


void TraceSegment::setPrefix(size_t pref) {
    prefix = pref;
}


 
vector<int> TraceSegment::getCopy() const {
    std::vector<int> w(segment.begin(), segment.begin() + std::min(prefix, segment.size()));
    return w;
}

Trace TraceSegment::getAsTrace(std::unique_ptr<FsmPresentationLayer> &&presentationLayer) const {
    return Trace(getCopy(), std::move(presentationLayer));
}

size_t TraceSegment::size() const {
    return std::min(prefix, segment.size());
}

int TraceSegment::at(size_t n) const {
    if (prefix != string::npos and prefix <= n) {
        return segment.at(segment.size());
    }
    return segment.at(n);
}

ostream & operator<<(ostream & out, const TraceSegment& seg)
{
    
    if ( seg.segment.empty() ) {
        out << "eps";
        return out;
    }
    
    out << seg.segment.at(0);
    
    for (size_t i = 1; i < seg.size(); i++ ) {
        out << "." << seg.segment.at(i);
    }
        
    return out;
    
}



// **************************************************************************

SegmentedTrace::SegmentedTrace(std::deque< TraceSegment > const &segments) {
    this->segments = segments;
}

SegmentedTrace::SegmentedTrace(const SegmentedTrace& other) {
    segments = other.segments;
}

void SegmentedTrace::add(TraceSegment const &seg) {
    segments.push_back(seg);
}

vector<int> SegmentedTrace::getCopy() const {
    vector<int> v;
    for ( auto &s : segments ) {
        vector<int> svec = s.getCopy();
        v.insert(v.end(),svec.begin(),svec.end());
    }
    return v;
}

FsmNode *SegmentedTrace::getTgtNode() const {
    if ( segments.empty() ) return nullptr;
    return segments.back().getTgtNode();
}

ostream & operator<<(ostream & out, const SegmentedTrace& trc) {
    if ( trc.segments.empty() ) {
        out << "eps";
        return out;
    }
    
    out << trc.segments.at(0);
    
    for ( size_t i = 1; i < trc.segments.size(); i++ ) {
        cout << "." << trc.segments[i];
    }
    return out;
    
}

bool operator==(SegmentedTrace const & trace1, SegmentedTrace const & trace2) {
    auto t1 = trace1.getCopy();
    auto t2 = trace2.getCopy();
    return t1 == t2;
}
