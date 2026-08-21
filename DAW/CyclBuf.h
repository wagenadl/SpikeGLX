// CyclBuf.h

#ifndef CYCLBUF_H

#define CYCLBUF_H

#include <vector>
#include <cstdint>
#include <cstring>

template <class T> class CyclBuf {
 public:
  CyclBuf(int log2size=16):
    stride(1) {
    int size = 1 << log2size;
    mask = size - 1;
    vec = std::vector<T>(size, 0);
    data = vec.data();
  }
  CyclBuf(T *data, int log2size, int stride=1):
    data(data), stride(stride) {
    int size = 1 << log2size;
    mask = size - 1;
  }
  T const &operator[](std::uint32_t index) const {
    index &= mask;
    return data[index*stride];
  }
  T &operator[](std::uint32_t index) {
    index &= mask;
    return data[index*stride];
  }
  void fill(T const *src, std::uint32_t n, std::uint32_t srcstride,
            std::uint32_t start, T sub=0) {
    std::uint32_t maskedstart = start & mask;
    std::uint32_t maskedend = (start + n) & mask;
    if (maskedend == 0 && n > 0)
      maskedend += mask + 1;
    if (maskedend > maskedstart) {
      // just a single span
      T *dst = data + maskedstart * stride;
      while (n > 0) {
        *dst = *src - sub;
        dst += stride;
        src += srcstride;
        --n;
      }
    } else {
      std::uint32_t m = n - maskedend; // this is how much goes in first span
      T *dst = data + maskedstart * stride;
      while (m > 0) {
        *dst = *src - sub;
        dst += stride;
        src += srcstride;
        --m;
      }
      dst = data;
      while (maskedend > 0) {
        *dst = *src - sub;
        dst += stride;
        src += srcstride;
        --maskedend;
      }
    }
  }
  void grab(T *dst, std::uint32_t n, std::uint32_t dststride,
            std::uint32_t start) const {
    std::uint32_t maskedstart = start & mask;
    std::uint32_t maskedend = (start + n) & mask;
    if (maskedend == 0 && n > 0)
      maskedend += mask + 1;
    if (maskedend > maskedstart) {
      // just a single span
      T const *src = data + maskedstart * stride;
      while (n > 0) {
        *dst = *src;
        dst += dststride;
        src += stride;
        --n;
      }
    } else {
      std::uint32_t m = n - maskedend; // this is how much goes in first span
      T const *src = data + maskedstart * stride;
      while (m > 0) {
        *dst = *src;
        dst += dststride;
        src += stride;
        --m;
      }
      src = data;
      while (maskedend > 0) {
        *dst = *src;
        dst += dststride;
        src += stride;
        --maskedend;
      }
    }
  }
    
  bool grow(int log2size) {
    // Copies old data into new space, replicated so old indices remain valid.
    // Fails if originally constructed with external data
    if (!vec.size()) // we don't own data, so no can do
      return false;
    std::uint32_t newsize = 1 << log2size;
    std::uint32_t oldsize = mask + 1;
    if (newsize <= oldsize)
      return true;
    
    vec.resize(newsize);
    for (std::uint32_t offset = oldsize; offset < newsize; offset += oldsize)
      std::memcpy(&vec[offset], &vec[0], oldsize*sizeof(T));
    mask = newsize - 1;
    data = &vec[0];
    return true;
  }
 private:
  std::vector<T> vec;
  T *data;
  std::uint32_t stride;
  std::uint32_t mask;
};

#endif
