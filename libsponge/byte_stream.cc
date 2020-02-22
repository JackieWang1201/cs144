#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <iostream>

// Implementation of a flow-controlled in-memory byte stream.

// Passes automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) {
    _capacity = capacity; 
    if(capacity==0)
        return;
    circular_ll = make_unique<Node>();
    std::shared_ptr<Node>* first = &circular_ll;//used to point last node's next to first node
    write_ptr = circular_ll.get();
    read_ptr  = circular_ll.get();//read_ptr to create capacity-1 more nodes
    for(int i =1; i < static_cast<int> ( capacity); i++)
    {
        read_ptr->next = make_unique<Node>();
        read_ptr = (read_ptr->next).get();
    }
    read_ptr->next = *first; // make the linked list circular
    read_ptr = write_ptr; //point read pointer to write point
    num_pop = 0, num_write = 0;
}

size_t ByteStream::write(const string &data) {
    size_t i = 0;
    size_t sz = data.size();
    while((buffer_empty()||(write_ptr!=read_ptr)) && i<sz)
    {
        write_ptr->val = data[i];
        write_ptr = (write_ptr->next).get();
        i++;
        num_write++;
    }
    return i;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string peek_data = "";
    size_t read_num = 0;
    Node* oread_ptr = (read_ptr);
    while((!buffer_empty() || (oread_ptr!=write_ptr)) && read_num < len)
    {
        peek_data = peek_data + (oread_ptr)->val;
        oread_ptr = (oread_ptr->next).get();
        read_num++;
    }
        
    return peek_data;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t read_num = 0;
    while((!buffer_empty() ||(read_ptr!=write_ptr)) && read_num < len)
    {
        read_ptr = (read_ptr->next).get();
        read_num++;
        num_pop++;
    }
}

void ByteStream::end_input() {_end_input = true;}

bool ByteStream::input_ended() const { return _end_input; }

size_t ByteStream::buffer_size() const { return num_write - num_pop; }

bool ByteStream::buffer_empty() const { return read_ptr==write_ptr && (remaining_capacity()>0); }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return num_write; }

size_t ByteStream::bytes_read() const { return num_pop ; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_size(); }
