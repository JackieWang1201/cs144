#include "stream_reassembler.hh"
#include <algorithm>


// passes automated checks run by `make check_lab1`.



StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity),
                                     last_assembled{-1}, total_bytes_rcvd{0}, buffer{}, intervals{}{
    buffer.resize(capacity);

}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t sz = data.size();
    size_t index_end = index + sz - 1;
    if(sz==0)
    {
        if(!eof_seen)
            eof_seen = eof; 
        if(eof_seen && unassembled_bytes()==0)
        {
            _output.end_input();
        }
        return;
    }
        
    //Insert first interval
    if(intervals.empty())
    {
        intervals.insert(intervals.end(),{index, index_end});
        transfer_to_buffer(index, index_end, index, data);
        if(!eof_seen)
            eof_seen = eof; 
        if(index==0)
        {
            _output.write(buffer.substr(0, sz));
            last_assembled = static_cast<int>(sz-1);
        }
        if(eof_seen && unassembled_bytes()==0)
        {
            _output.end_input();
        }
        return;
    }


    auto it = intervals.begin();
    auto last_assembled_it = intervals.end();
    //Iterate through intervals which are less than the new intervals
    while(it!=intervals.end() && it->second < index)
    {
        it++;
    }
    //If new interval is part of an existing interval
    if(it!=intervals.end() && it->first<=index && index_end<=it->second)
        it++;
    //If new interval does not overlap with existing interval
    else if((it!=intervals.end() && index_end < it->first) || it==intervals.end())
    {
        transfer_to_buffer(index, index_end, index, data);
        size_t interval_start = index;
        auto prev = it;
        if(it!=intervals.begin())
            prev--;

        if( it!=intervals.begin() && index==(prev)->second+1)
        {
            it = prev;
            it->second = index_end;
        }
        else
            it = intervals.insert(it, {interval_start, index_end});
        auto next = it;
        if(it!=intervals.end())
            next++;
        if(it!=intervals.end() && next!=intervals.end() && (index_end+1) == next->first)
        {   
            it->second = next->second;
            if(it->first<=static_cast<size_t>(last_assembled+1) && static_cast<size_t>(last_assembled+1)<=it->second)
            {
                last_assembled_it = it;
            }
            it = intervals.erase(next);
        }
        else
        {
            if(it->first<=static_cast<size_t>(last_assembled+1) && static_cast<size_t>(last_assembled+1)<=it->second)
            {
                last_assembled_it = it;
            }
            it++;
        }
            

    }    
    else
    {
        size_t interval_start, seen_till_now;
        auto it_sec = it;
        auto prev = it;
        if(it!=intervals.begin())
            prev--;
        auto next = it;
        //Example: Existing interval (5,7); New interval (2,3)
        if(it!=intervals.end() && index < it->first)
        {   
            interval_start = index;
            transfer_to_buffer(index, it->first-1, index, data);
            //Example: Existing interval (0,1), (5,7); New interval (2,3)
            if(it!=intervals.begin() && index==(prev)->second+1)
            {
                next = it;
                it = prev;
                it->second = next->second;
            }
            else
            {
                next = it;
                it = intervals.insert(it, {interval_start, next->second});
            }
            seen_till_now = next->second;
            it_sec = intervals.erase(next);
        }
        //Example: Existing interval (5,7); New interval (6,8)
        else
        {
            seen_till_now = next->second;
            it_sec++;
        }
        while(it_sec!=intervals.end() && it_sec->first <= index_end )
        {
            if(it_sec->first > 0)
                transfer_to_buffer(seen_till_now+1, it_sec->first-1, index, data);
            seen_till_now = it_sec->second;
            it_sec = intervals.erase(it_sec);
        }
        if(seen_till_now < index_end)
        {
            transfer_to_buffer(seen_till_now+1, index_end, index, data);
            seen_till_now = index_end;
        }
        if(it_sec!=intervals.end() && seen_till_now+1 == it_sec->first)
        {
            it->second = it_sec->second;
            it_sec = intervals.erase(it_sec);
        }
        else
        {
            it->second = seen_till_now;
        }
        if(it->first<=static_cast<size_t>(last_assembled+1) && static_cast<size_t>(last_assembled+1)<=it->second)
            last_assembled_it = it;
    }
            
        
            

    if(!eof_seen)
        eof_seen = eof; 
    if(last_assembled_it!=intervals.end())
    {
        string subs(buffer.cbegin() + last_assembled+1, buffer.cbegin() + static_cast<int>(last_assembled_it->second) + 1);
        _output.write(move(subs));
        last_assembled = static_cast<int>(last_assembled_it->second);
    }
    if(eof_seen && unassembled_bytes()==0)
    {
        _output.end_input();
    }
            
        
}

void StreamReassembler::transfer_to_buffer(size_t start, size_t end, size_t index, const string & data){
    size_t bi;
    size_t bi_limit = min(end, _capacity-1);
    for(bi = start; bi<=bi_limit ; ++bi)
    {
        buffer[bi] = data[bi-index];
    }
    total_bytes_rcvd+= bi-start;
}

size_t StreamReassembler::unassembled_bytes() const { return total_bytes_rcvd - (last_assembled + 1); }

bool StreamReassembler::empty() const { return (unassembled_bytes()==0); }
