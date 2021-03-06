#ifndef EXTENDEDSUBSTITUIONMATRIXH
#define EXTENDEDSUBSTITUIONMATRIXH
#include <algorithm>
#include <string>
#include <vector>

#include <biosnake/output.h>
#include <biosnake/commons/baseMatrix.h>
#include <biosnake/commons/scoreMatrix.h>  // ScoreMatrix

class ExtendedSubstitutionMatrix {
 public:
  static ScoreMatrix calcScoreMatrix(biosnake_output* out, const BaseMatrix& matrix,
                                     const size_t kmerSize);
  static void freeScoreMatrix(ScoreMatrix& matrix);

  static short calcScore(biosnake_output* out, unsigned char* i_seq, unsigned char* j_seq,
                         size_t seq_size, short** subMatrix);

 private:
  static std::vector<std::vector<unsigned char> > buildInput(size_t dimension,
                                                             size_t range);

  static void createCartesianProduct(
      std::vector<std::vector<unsigned char> >& output,  // final result
      std::vector<unsigned char>& current_result,        // current result
      std::vector<std::vector<unsigned char> >::const_iterator
          current_input,  // current input
      std::vector<std::vector<unsigned char> >::const_iterator
          end);  // final input
};
#endif
