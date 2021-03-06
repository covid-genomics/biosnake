#ifndef BIOSNAKE_AGGREGATION_H
#define BIOSNAKE_AGGREGATION_H

#include <biosnake/commons/dBReader.h>
#include <biosnake/commons/dBWriter.h>

#include <map>
#include <vector>

class Aggregation {
 public:
  Aggregation(biosnake_output* output, const std::string &targetDbName, const std::string &resultDbName,
              const std::string &outputDbName, unsigned int threads,
              unsigned int compressed);

  virtual ~Aggregation();

  int run();
  virtual void prepareInput(unsigned int querySetKey,
                            unsigned int thread_idx) = 0;
  virtual std::string aggregateEntry(
      std::vector<std::vector<std::string>> &dataToAggregate,
      unsigned int querySetKey, unsigned int targetSetKey,
      unsigned int thread_idx) = 0;

 protected:
  biosnake_output* out;
  std::string resultDbName;
  std::string outputDbName;
  DBReader<unsigned int> *targetSetReader;
  unsigned int threads;
  unsigned int compressed;

  void buildMap(char *data, int thread_idx,
                std::map<unsigned int, std::vector<std::vector<std::string>>>
                    &dataToAggregate);
};

#endif
