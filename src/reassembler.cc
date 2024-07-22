#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  uint64_t stringMaxIndex = first_index+data.size();
  uint64_t bufMaxIndex = curIndex + writer().available_capacity();
  if (first_index >= bufMaxIndex || stringMaxIndex < curIndex) {
    return;
  }

  //If this is the last piece of data, we should store the data starting position so that the
  //reassembler knows when to end in the future.
  if (is_last_substring)
    eofPos = first_index + data.size();

  //The data can be pushed immediately
  if (first_index <= curIndex) {
    uint64_t startIndex = curIndex - first_index;//The first push to bytestream data position
    uint64_t endIndex = stringMaxIndex > bufMaxIndex ? data.size()-(stringMaxIndex-bufMaxIndex) : data.size();
    string pushData;
    for (; startIndex < endIndex; startIndex++) {
      pushData.push_back(data.at(startIndex));
      storage.erase(curIndex++);
    }

    while (storage.contains(curIndex)) {//Check whether the previous stored string can be put together
      pushData.push_back(storage.at(curIndex));
      storage.erase(curIndex);
      curIndex ++;
    }

    if (!pushData.empty())//Push the data to the bytestream
      output_.writer().push(pushData);

    if (curIndex >= eofPos) {//If there is an eof mark along the contiguous string we just pushed, close the byte stream
      output_.writer().close();
    }
    return;
  }

  //The data cannot be pushed immediately as previous data is unknown.
  uint64_t endIndex = stringMaxIndex <= bufMaxIndex ? first_index+data.size() : bufMaxIndex;
  for (char c : data) {
    if (first_index >= endIndex)
      break;
    if (storage.contains(first_index)) {
      continue;
    }
    storage.insert( { first_index, c } );
    first_index ++;
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return storage.size();
}
