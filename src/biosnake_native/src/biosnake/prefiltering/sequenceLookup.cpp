//
// Created by mad on 12/14/15.
//
#include <biosnake/prefiltering/sequenceLookup.h>
#include <sys/mman.h>
#include <cstring>
#include <new>
#include <biosnake/output.h>
#include <biosnake/commons/util.h>

SequenceLookup::SequenceLookup(biosnake_output* output, size_t sequenceCount, size_t dataSize)
    : out(output), sequenceCount(sequenceCount),
      dataSize(dataSize),
      currentIndex(0),
      currentOffset(0),
      externalData(false) {
  data = new (std::nothrow) char[dataSize + 1];
  Util::checkAllocation(out, data, "Can not allocate data memory in SequenceLookup");

  offsets = new (std::nothrow) size_t[sequenceCount + 1];
  Util::checkAllocation(out, offsets,
                        "Can not allocate offsets memory in SequenceLookup");
  offsets[sequenceCount] = dataSize;
}

SequenceLookup::SequenceLookup(biosnake_output* output, size_t sequenceCount)
    : out(output),
      sequenceCount(sequenceCount),
      data(NULL),
      dataSize(0),
      offsets(NULL),
      currentIndex(0),
      currentOffset(0),
      externalData(true) {}

SequenceLookup::~SequenceLookup() {
  if (externalData == false) {
    delete[] data;
    delete[] offsets;
  }
}

void SequenceLookup::addSequence(unsigned char *seq, int L, size_t index,
                                 size_t offset) {
  offsets[index] = offset;
  memcpy(&data[offset], seq, L);
}

void SequenceLookup::addSequence(Sequence *seq) {
  addSequence(seq->numSequence, seq->L, currentIndex, currentOffset);
  currentIndex = currentIndex + 1;
  currentOffset = currentOffset + seq->L;
}

std::pair<const unsigned char *, const unsigned int>
SequenceLookup::getSequence(size_t id) {
  ptrdiff_t N = offsets[id + 1] - offsets[id];
  char *p = data + offsets[id];
  return std::pair<const unsigned char *, const unsigned int>(
      reinterpret_cast<const unsigned char *>(p), static_cast<unsigned int>(N));
}

const char *SequenceLookup::getData() { return data; }

int64_t SequenceLookup::getDataSize() { return dataSize; }

size_t *SequenceLookup::getOffsets() { return offsets; }

size_t SequenceLookup::getSequenceCount() { return sequenceCount; }

void SequenceLookup::initLookupByExternalData(char *seqData, size_t seqDataSize,
                                              size_t *seqOffsets) {
  dataSize = seqDataSize;

  data = seqData;
  offsets = seqOffsets;
}

void SequenceLookup::initLookupByExternalDataCopy(char *seqData,
                                                  size_t *seqOffsets) {
  memcpy(data, seqData, (dataSize + 1) * sizeof(char));
  memcpy(offsets, seqOffsets, (sequenceCount + 1) * sizeof(size_t));
}
