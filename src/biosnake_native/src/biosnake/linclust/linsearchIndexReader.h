#ifndef BIOSNAKE_INDEXKMERDB_H
#define BIOSNAKE_INDEXKMERDB_H

#include <biosnake/linclust/kmermatcher.h>

struct FileKmer {
  size_t kmer;
  unsigned int id;
  unsigned int file;
  unsigned short seqLen;
  short pos;
  bool reverse;
  FileKmer() {}
  FileKmer(size_t kmer, unsigned int id, short pos, short seqLen, bool reverse,
           unsigned int file)
      : kmer(kmer),
        id(id),
        file(file),
        seqLen(seqLen),
        pos(pos),
        reverse(reverse) {}
};

class CompareRepSequenceAndIdAndDiag {
 public:
  bool operator()(FileKmer &first, FileKmer &second) const {
    if (first.kmer > second.kmer) return true;
    if (second.kmer > first.kmer) return false;
    if (first.id > second.id) return true;
    if (second.id > first.id) return false;
    if (first.pos > second.pos) return true;
    if (second.pos > first.pos) return false;
    return false;
  }
};

class LinsearchIndexReader {
 public:
  template <int TYPE>
  static size_t pickCenterKmer(biosnake_output* out, KmerPosition<short> *kmers,
                               size_t splitKmerCount);

  template <int TYPE>
  static void mergeAndWriteIndex(biosnake_output* out, DBWriter &dbw,
                                 std::vector<std::string> tmpFiles,
                                 int alphSize, int kmerSize);

  template <int TYPE>
  static void writeIndex(biosnake_output* out, DBWriter &dbw, KmerPosition<short> *hashSeqPair,
                         size_t totalKmers, int alphSize, int kmerSize);

  static std::string indexName(biosnake_output* out, std::string baseName);

  static void writeKmerIndexToDisk(biosnake_output* out, std::string fileName,
                                   KmerPosition<short> *kmers, size_t kmerCnt);

  static bool checkIfIndexFile(biosnake_output* out, DBReader<unsigned int> *pReader);

  static std::string findIncompatibleParameter(biosnake_output* out, DBReader<unsigned int> &index,
                                               Parameters &parameters,
                                               int dbtype);

  static std::string searchForIndex(biosnake_output* out, const std::string &dbName);
};
#endif
