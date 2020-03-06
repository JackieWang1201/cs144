#include "tcp_receiver.hh"

// passes automated checks run by `make check_lab2`.


using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    bool old_is_syn_seen = is_syn_seen;
    bool old_is_fin_seen = is_fin_seen;
    TCPHeader segment_header = seg.header();
    if(!is_syn_seen && !segment_header.syn)
        return false;
    if(!is_syn_seen)
    {
        is_syn_seen = true;
        _isn = segment_header.seqno+1;
    }
    if(!is_fin_seen && segment_header.fin)
    {
        is_fin_seen = true;
        _reassembler.stream_out().end_input();
    }
    //Incoming segment start,end range
    uint64_t index_start = unwrap(segment_header.seqno, _isn, _checkpoint);
    uint64_t index_end = index_start + seg.length_in_sequence_space() - 1;
    if(index_end < index_start)   //Bare acknowledgement
        index_end++;
    //Acceptable start,end range
    uint64_t seqno_start = unwrap(ackno().value(), _isn, _checkpoint);
    uint64_t seqno_end = seqno_start + window_size() - 1;
    if(seqno_end < seqno_start)    //Window_size = 0
        seqno_end++;         
    _reassembler.push_substring(seg.payload().copy(), index_start, segment_header.fin);

    _checkpoint = _reassembler.stream_out().bytes_written();
    _ackno =  wrap(_reassembler.stream_out().bytes_written() + (segment_header.fin?1:0) , _isn);
    if((index_start >= seqno_start && index_start <= seqno_end )  || (index_end >= seqno_start && index_end <=seqno_end) ||
            //Reject second SYN or second FIN
            (segment_header.fin && !old_is_fin_seen) || (segment_header.syn && !old_is_syn_seen)) 
        return true;
    return false;
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(is_syn_seen)
        return _ackno ;
    return {};
 }

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
