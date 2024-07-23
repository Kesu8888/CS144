#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return eof;
}

void Writer::push( string data )
{
  // Your code here.
  uint64_t size = data.size()+buf.size() > capacity_ ? capacity_-buf.size() : data.size();
  bytesPushed += size;
  buf.append(string_view(data.data(), size));
}

void Writer::close()
{
  // Your code here.
    eof = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_-buf.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytesPushed;
}

bool Reader::is_finished() const
{
  // Your code here.
  return buf.empty() && eof;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytesPoped;
}

string_view Reader::peek() const
{
  // Your code here.
  return buf;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  bytesPoped += len;
  buf = string_view(buf.data()+len, buf.size()-len);
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buf.size();
}
