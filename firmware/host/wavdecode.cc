// Decodes a firmware-update WAV with the real Plaits bootloader demodulator.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "stm_audio_bootloader/qpsk/packet_decoder.h"
#include "stm_audio_bootloader/qpsk/demodulator.h"
using namespace stm_audio_bootloader;
const uint32_t PAGE = 2048;
int main(int argc, char** argv) {
  FILE* f = fopen(argv[1], "rb"); fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
  std::vector<unsigned char> w(n); fread(&w[0], 1, n, f); fclose(f);
  const int16_t* s = (const int16_t*)(&w[44]); long ns = (n - 44) / 2;
  float gain = argc > 3 ? atof(argv[3]) : 1.0f;
  int invert = argc > 4 ? atoi(argv[4]) : 0;
  static PacketDecoder decoder; static Demodulator demodulator;
  decoder.Init(1000, true);
  demodulator.Init(6000.0 / 48000.0 * 4294967296.0, 48000.0 / 6000.0, 2.0 * 48000.0 / 12000.0);
  demodulator.SyncCarrier(true);
  decoder.Reset();
  std::vector<uint8_t> out; uint8_t page[PAGE]; int packet_index = 0; bool done = false; int errors = 0;
  int discard = 8000;
  for (long i = 0; i < ns && !done; ++i) {
    int32_t sample = (int32_t)(s[i] * gain) * (invert ? -1 : 1);
    if (sample > 32767) sample = 32767; if (sample < -32767) sample = -32767;
    if (discard) { --discard; continue; }
    demodulator.PushSample(2048 + (sample >> 4));
    if ((i & 31) == 0 || i == ns - 1) {
      if (demodulator.state() == DEMODULATOR_STATE_OVERFLOW) { printf("overflow\n"); return 2; }
      demodulator.ProcessAtLeast(32);
      while (demodulator.available() && !done) {
        PacketDecoderState st = decoder.ProcessSymbol(demodulator.NextSymbol());
        if (st == PACKET_DECODER_STATE_OK) {
          memcpy(page + (packet_index % (PAGE / kPacketSize)) * kPacketSize, decoder.packet_data(), kPacketSize);
          ++packet_index;
          if (packet_index % (PAGE / kPacketSize) == 0) {
            out.insert(out.end(), page, page + PAGE);
            decoder.Reset(); demodulator.SyncCarrier(false);
          } else { decoder.Reset(); demodulator.SyncDecision(); }
        } else if (st == PACKET_DECODER_STATE_ERROR_CRC || st == PACKET_DECODER_STATE_ERROR_SYNC) {
          printf("decoder error %d after %d packets\n", st, packet_index); errors++; done = true;
        } else if (st == PACKET_DECODER_STATE_END_OF_TRANSMISSION) { done = true; }
      }
    }
  }
  FILE* b = fopen(argv[2], "rb"); fseek(b, 0, SEEK_END); long bn = ftell(b); fseek(b, 0, SEEK_SET);
  std::vector<uint8_t> bin(bn); fread(&bin[0], 1, bn, b); fclose(b);
  bool match = out.size() >= (size_t)bn && memcmp(&out[0], &bin[0], bn) == 0;
  bool pad_ok = true; for (size_t i = bn; i < out.size(); ++i) pad_ok &= out[i] == 0xff;
  printf("decoded %zu bytes in %d packets, firmware %ld bytes: %s%s\n", out.size(), packet_index, bn, match ? "IDENTICAL" : "MISMATCH", pad_ok ? "" : " (bad padding)");
  return match && !errors ? 0 : 1;
}
