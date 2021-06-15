
#include <cstring>

#include <tjpgd.h>

namespace {

/* User defined session identifier */
struct IODEV {
  IODEV(const uint8_t* input_data, size_t input_data_size)
      : input_data(input_data), input_data_size(input_data_size), pos(0) {}

  const uint8_t* input_data;     // JPEG data.
  const size_t input_data_size;  // Size of |input_data|.
  size_t pos;                    // Current read position in |input_data|.
};

/**
 * TJpgDec input function.
 *
 * @param jd Decompression object.
 * @param buff Pointer to the read buffer (null to remove data).
 * @param ndata Number of bytes to read/skip.
 *
 * @return number of bytes read (zero on error).
 */
size_t input_func(JDEC* jd, uint8_t* buff, size_t ndata) {
  IODEV* dev = static_cast<IODEV*>(jd->device);

  const size_t bytes_left = dev->input_data_size - dev->pos;
  const size_t to_read = ndata <= bytes_left ? ndata : bytes_left;
  if (to_read == 0)
    return 0;

  if (buff)
    std::memcpy(buff, dev->input_data + dev->pos, to_read);

  dev->pos += to_read;
  return to_read;
}

/**
 * TJpgDec output function.
 *
 * @param jd Decompression object.
 * @param bitmap Bitmap data to be output.
 * @param rect Rectangular region to output.
 *
 * @return 1:Ok, 0:Aborted.
 */
int output_func(JDEC* jd, void* bitmap, JRECT* rect) {
  return 1;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  constexpr size_t kWorkBufferSize = 4096;
  constexpr uint8_t SCALE = 0;

  uint8_t work_buffer[kWorkBufferSize];
  JDEC decompressor;
  IODEV iodev(data, size);

  JRESULT rc = jd_prepare(&decompressor, input_func, work_buffer,
                          kWorkBufferSize, &iodev);
  // assert(rc == JDR_OK);
  jd_decomp(&decompressor, output_func, SCALE);
  return 0;
}
