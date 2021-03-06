#include <biosnake/prefiltering/indexer.h>
#include <biosnake/output.h>

Indexer::Indexer(biosnake_output* output, const size_t alphabetSize, const int maxKmerSize): out(output) {
  this->maxKmerSize = maxKmerSize;
  this->powers = new size_t[maxKmerSize];
  this->alphabetSize = alphabetSize;
  size_t pow = 1;
  for (int i = 0; i < maxKmerSize; ++i) {
    this->powers[i] = pow;
    pow *= alphabetSize;
  }
  this->maxKmerIndex = 0;
  for (int i = 0; i < maxKmerSize; ++i) {
    this->maxKmerIndex += alphabetSize * this->powers[i];
  }

  this->lastKmerIndex = this->maxKmerIndex;

  workspace = new size_t[100];
}

Indexer::~Indexer() {
  delete[] this->powers;
  delete[] workspace;
}

void Indexer::reset() { this->lastKmerIndex = this->maxKmerIndex; }

void Indexer::printKmer(size_t kmerIdx, int kmerSize, char* num2aa) {
  index2int(workspace, kmerIdx, kmerSize);
  for (int j = 0; j < kmerSize; j++) {
    out->info("{}", num2aa[workspace[j]]);
  }
}

void Indexer::printKmer(const unsigned char* kmer, int kmerSize, char* num2aa) {
  for (int j = 0; j < kmerSize; j++) {
    out->info("{}", num2aa[kmer[j]]);
  }
}
