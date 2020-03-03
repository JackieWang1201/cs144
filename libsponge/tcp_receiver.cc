#include "tcp_receiver.hh"


// passes automated checks run by `make check_lab2`.


using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    bool old_is_syn_seen = is_syn_seen;
    bool old_is_fin_seen = is_fin_seen;
    size_t unassembled_bytes_old, unassembled_bytes_new;
    size_t buffer_size_old, buffer_size_new;
    unassembled_bytes_old = _reassembler.unassembled_bytes();
    buffer_size_old = _reassembler.stream_out().bytes_written();
    TCPHeader segment_header = seg.header();
    if(!is_syn_seen && !segment_header.syn)
        return false;
    if(!is_syn_seen)
    {
        is_syn_seen = true;
        _isn = segment_header.seqno+1;
    }
    if(!is_fin_seen && segment_header.fin)
        is_fin_seen = true;
    uint64_t index_start = unwrap(segment_header.seqno, _isn, _checkpoint);
    _reassembler.push_substring(seg.payload().copy(), index_start, segment_header.fin);
    _checkpoint = _reassembler.stream_out().bytes_written();
    unassembled_bytes_new = _reassembler.unassembled_bytes();
    buffer_size_new = _reassembler.stream_out().bytes_written();
    _ackno = _isn + (_reassembler.stream_out().bytes_written() + (segment_header.fin?1:0) );
    if(unassembled_bytes_new > unassembled_bytes_old || buffer_size_new > buffer_size_old||
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
