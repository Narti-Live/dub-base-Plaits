#!/usr/bin/env python3
# Python 3 port of stm_audio_bootloader/qpsk/encoder.py
# Copyright 2013 Emilie Gillet (MIT license). Port 2026.
#
# Usage (same settings as the Plaits makefile 'wav' target):
#   qpsk_encoder3.py -t stm32f3 -k -s 48000 -b 12000 -c 6000 -p 256 firmware.bin -o firmware.wav

import argparse
import struct
import wave
import zlib

import numpy


class Scrambler(object):
    def __init__(self, seed=0):
        self._state = seed

    def scramble(self, data):
        out = bytearray(data)
        for i in range(len(out)):
            out[i] ^= (self._state >> 24) & 0xff
            self._state = (self._state * 1664525 + 1013904223) & 0xffffffff
        return bytes(out)


class QpskEncoder(object):
    def __init__(self, sample_rate=48000, carrier_frequency=2000, bit_rate=8000,
                 packet_size=256, scramble=False):
        period = sample_rate // carrier_frequency
        symbol_time = sample_rate // (bit_rate // 2)
        assert (symbol_time % period == 0) or (period % symbol_time == 0)
        assert (sample_rate % carrier_frequency) == 0
        assert (sample_rate % bit_rate) == 0
        self._sr = sample_rate
        self._br = bit_rate
        self._carrier_frequency = carrier_frequency
        self._sample_index = 0
        self._packet_size = packet_size
        self._scrambler = Scrambler(0) if scramble else None

    @staticmethod
    def _upsample(x, factor):
        return numpy.tile(x.reshape(len(x), 1), (1, factor)).ravel()

    def _encode_qpsk(self, symbol_stream):
        ratio = self._sr // self._br * 2
        symbol_stream = numpy.array(symbol_stream, dtype=int)
        bitstream_even = 2 * self._upsample(symbol_stream % 2, ratio) - 1
        bitstream_odd = 2 * self._upsample(symbol_stream // 2, ratio) - 1
        return bitstream_even / numpy.sqrt(2.0), bitstream_odd / numpy.sqrt(2.0)

    def _modulate(self, q_mod, i_mod):
        num_samples = len(q_mod)
        t = (numpy.arange(0.0, num_samples) + self._sample_index) / self._sr
        self._sample_index += num_samples
        phase = 2 * numpy.pi * self._carrier_frequency * t
        return q_mod * numpy.sin(phase) + i_mod * numpy.cos(phase)

    def _encode(self, symbol_stream):
        return self._modulate(*self._encode_qpsk(symbol_stream))

    def _code_blank(self, duration):
        num_zeros = int(duration * self._br / 8) * 4
        return self._encode(numpy.zeros(num_zeros, dtype=int))

    def _code_packet(self, data):
        assert len(data) <= self._packet_size
        if len(data) != self._packet_size:
            data = data + b'\xff' * (self._packet_size - len(data))
        if self._scrambler:
            data = self._scrambler.scramble(data)
        crc = zlib.crc32(data) & 0xffffffff
        preamble = [0] * 8 + [0x99] * 4 + [0xcc] * 4
        crc_bytes = [crc >> 24, (crc >> 16) & 0xff, (crc >> 8) & 0xff, crc & 0xff]
        all_bytes = preamble + list(data) + crc_bytes
        symbols = []
        for byte in all_bytes:
            symbols.append((byte >> 6) & 0x3)
            symbols.append((byte >> 4) & 0x3)
            symbols.append((byte >> 2) & 0x3)
            symbols.append((byte >> 0) & 0x3)
        return self._encode(symbols)

    def code_intro(self):
        yield numpy.zeros(1 * self._sr)
        yield self._code_blank(1.0)

    def code_outro(self, duration=1.0):
        yield self._code_blank(duration)

    def code(self, data, page_size=1024, blank_duration=0.06):
        if len(data) % page_size != 0:
            data += b'\xff' * (page_size - (len(data) % page_size))
        offset = 0
        remaining = len(data)
        written = 0
        while remaining:
            size = min(remaining, self._packet_size)
            yield self._code_packet(data[offset:offset + size])
            written += 1
            if written == page_size // self._packet_size:
                yield self._code_blank(blank_duration)
                written = 0
            remaining -= size
            offset += size


PAGE_SIZE = {'stm32f1': 1024, 'stm32f3': 2048}
PAUSE = {'stm32f1': 0.06, 'stm32f3': 0.15}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('bin')
    ap.add_argument('-k', '--scramble', action='store_true')
    ap.add_argument('-s', '--sample_rate', type=int, default=48000)
    ap.add_argument('-c', '--carrier_frequency', type=int, default=6000)
    ap.add_argument('-b', '--baud_rate', type=int, default=12000)
    ap.add_argument('-p', '--packet_size', type=int, default=256)
    ap.add_argument('-t', '--target', default='stm32f1')
    ap.add_argument('-o', '--output_file')
    a = ap.parse_args()
    data = open(a.bin, 'rb').read()
    enc = QpskEncoder(a.sample_rate, a.carrier_frequency, a.baud_rate, a.packet_size, a.scramble)
    blocks = list(enc.code_intro())
    blocks += [b for b in enc.code(data, PAGE_SIZE[a.target], PAUSE[a.target]) if len(b)]
    blocks += list(enc.code_outro(1.0))
    signal = numpy.concatenate(blocks)
    peak = numpy.abs(signal).max()
    if peak > 1.0:
        signal = numpy.clip(signal, -1, 1)
    pcm = (signal * 32767).astype('<i2')
    out = a.output_file or a.bin.replace('.bin', '.wav')
    with wave.open(out, 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(a.sample_rate)
        w.writeframes(pcm.tobytes())
    print('wrote %s: %.1f s' % (out, len(pcm) / a.sample_rate))


if __name__ == '__main__':
    main()
