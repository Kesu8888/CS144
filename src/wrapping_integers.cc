#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return zero_point + (uint32_t)(n % (1LL << 32));
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint64_t seqno = zero_point.raw_value_ <= raw_value_ ? (raw_value_-zero_point.raw_value_)
                                                       : (UINT32_MAX-(zero_point.raw_value_-raw_value_)+1);
  uint64_t absSeqno = checkpoint % (1LL << 32);
  if (checkpoint == 0 || absSeqno == seqno)
    return checkpoint == 0 ? seqno : checkpoint;
  if (absSeqno > seqno) {
    uint64_t offset = checkpoint/(1LL << 32)+1;
    absSeqno = ((absSeqno-seqno) <= (1LL << 32)/2 ? (offset-1) : offset)*(1LL << 32) + seqno;
  } else {
    uint64_t offset = checkpoint/(1LL << 32);
    absSeqno = ((seqno - absSeqno) <= (1LL << 32)/2 ? offset : offset-1)*(1LL << 32) + seqno;
  }
  return absSeqno;
}
