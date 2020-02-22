#include "stream_reassembler.hh"
#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity),
                                     last_assembled{-1}, total_bytes_rcvd{0}, buffer{}, index_seen {}{
    buffer.resize(capacity);

    //cerr<<"Capacity: "<<capacity<<"\n";
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t sz = data.size();

    //cerr<<sz<<" "<<index<<" "<<eof<<"\n";
    size_t i =0, j = index;
    for(; i<sz && j< _capacity; i++, j++)
    {
        //if(data[i]=='\0')
        //    cerr<<"Warning\n\n\n";
        buffer[j] = data[i];
        if(!index_seen.count(j))
        {
            total_bytes_rcvd++;
        }
        index_seen.insert(j);
    }
    /*for(auto x:buffer)
        cerr<<x;
    cerr<<"------\n";*/
    if(!eof_seen)
        eof_seen = eof; 
    if((last_assembled+1)<static_cast<int>(_capacity) && index_seen.count(last_assembled+1))
    {
        //string to_be_streamed = "";
        i = last_assembled+1;
        while(i<_capacity && index_seen.count(i))
        {   
            //to_be_streamed+=buffer[i];
            //last_assembled++;
            i++;
        }
        _output.write(buffer.substr(last_assembled+1, i-last_assembled-1));
        last_assembled = i-1;
    }
    //cerr<<unassembled_bytes()<<" "<<last_assembled<<"\n";
    if(eof_seen && unassembled_bytes()==0)
    {
        _output.end_input();
        //cerr<<total_bytes_rcvd<<"\n";
    }
            
        
}

size_t StreamReassembler::unassembled_bytes() const { return total_bytes_rcvd - (last_assembled + 1); }

bool StreamReassembler::empty() const { return (unassembled_bytes()==0); }
