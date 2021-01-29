//
// Created by mad on 10/26/15.
//
#include <iostream>

#include "parameters.h"
#include "sequence.h"
#include "substitutionMatrix.h"

const char* binary_name = "test_kmerscore";

int main(int, const char**) {
  const size_t kmer_size = 6;

  Parameters& par = Parameters::getInstance();
  SubstitutionMatrix subMat(par.scoringMatrixFile.aminoacids, 8.0, 0);
  std::cout << "Subustitution matrix:\n";
  SubstitutionMatrix::print(subMat.subMatrix, subMat.num2aa,
                            subMat.alphabetSize);

  const char* ref = "GKILII";
  Sequence refSeq(1000, 0, &subMat, kmer_size, false, true);
  refSeq.mapSequence(0, 0, ref, strlen(ref));

  const char* similar = "GKVLYL";
  Sequence similarSeq(1000, 0, &subMat, kmer_size, false, true);
  similarSeq.mapSequence(0, 1, similar, strlen(similar));

  short score = 0;
  for (size_t i = 0; i < kmer_size; i++) {
    score += subMat.subMatrix[refSeq.numSequence[i]][similarSeq.numSequence[i]];
  }
  std::cout << score << std::endl;

  return 0;
}
