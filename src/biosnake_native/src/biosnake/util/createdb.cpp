/*
 * createdb
 * written by Martin Steinegger <martin.steinegger@snu.ac.kr>.
 * modified by Maria Hauser <mhauser@genzentrum.lmu.de> (splitting into
 * sequences/headers databases) modified by Milot Mirdita <milot@mirdita.de>
 */

#include <biosnake/commons/dBWriter.h>
#include <biosnake/output.h>
#include <biosnake/commons/fileUtil.h>
#include <biosnake/commons/kSeqWrapper.h>
#include <biosnake/commons/util.h>
#include <biosnake/commons/itoa.h>
#include <biosnake/output.h>

int createdb(biosnake_output* out, Parameters& par) {
std::cout << "CMDDEBUG biosnake createdb " << par.createParameterString(out, par.createdb);

  //    Parameters &par = Parameters::getInstance();
  //    par.parseParameters(argc, argv, command, true,
  //    Parameters::PARSE_VARIADIC, 0);

  std::vector<std::string> filenames(par.filenames);
  std::string dataFile = filenames.back();
  filenames.pop_back();

  for (size_t i = 0; i < filenames.size(); i++) {
    if (FileUtil::directoryExists(out, filenames[i].c_str()) == true) {
      out->failure("File {} is a directory", filenames[i] );
    }
  }

  bool dbInput = false;
  if (FileUtil::fileExists(out, par.db1dbtype.c_str()) == true) {
    if (filenames.size() > 1) {
      out->failure("Only one database can be used with database input");
    }
    dbInput = true;
    par.createdbMode = Parameters::SEQUENCE_SPLIT_MODE_HARD;
  }

  int dbType = -1;
  if (par.dbType == 2) {
    dbType = Parameters::DBTYPE_NUCLEOTIDES;
  } else if (par.dbType == 1) {
    dbType = Parameters::DBTYPE_AMINO_ACIDS;
  }

  std::string indexFile = dataFile + ".index";
  if (par.createdbMode == Parameters::SEQUENCE_SPLIT_MODE_SOFT &&
      par.shuffleDatabase) {
    out->warn("Shuffle database cannot be combined with --createdb-mode 0. We recompute with --shuffle 0");
    par.shuffleDatabase = false;
  }

  if (par.createdbMode == Parameters::SEQUENCE_SPLIT_MODE_SOFT &&
      par.filenames[0] == "stdin") {
    out->warn("Stdin input cannot be combined with --createdb-mode 0. We recompute with --createdb-mode 1");
    par.createdbMode = Parameters::SEQUENCE_SPLIT_MODE_HARD;
  }

  const unsigned int shuffleSplits = par.shuffleDatabase ? 32 : 1;
  if (par.createdbMode == Parameters::SEQUENCE_SPLIT_MODE_SOFT &&
      par.compressed) {
    out->warn("Compressed database cannot be combined with --createdb-mode 0. We recompute with --compressed 0");
    par.compressed = 0;
  }

  std::string hdrDataFile = dataFile + "_h";
  std::string hdrIndexFile = dataFile + "_h.index";

  unsigned int entries_num = 0;
  size_t sampleCount = 0;

  const char newline = '\n';

  const size_t testForNucSequence = 100;
  size_t isNuclCnt = 0;
  Log::Progress progress;
  std::vector<unsigned short>* sourceLookup =
      new std::vector<unsigned short>[shuffleSplits]();
  for (size_t i = 0; i < shuffleSplits; ++i) {
    sourceLookup[i].reserve(16384);
  }
  out->info("Converting sequences");

  std::string sourceFile = dataFile + ".source";

redoComputation:
  FILE* source = fopen(sourceFile.c_str(), "w");
  if (source == NULL) {
    out->failure("Cannot open {} for writing", sourceFile);
  }
  DBWriter hdrWriter(out, hdrDataFile.c_str(), hdrIndexFile.c_str(), shuffleSplits,
                     par.compressed, Parameters::DBTYPE_GENERIC_DB);
  hdrWriter.open();
  DBWriter seqWriter(out, dataFile.c_str(), indexFile.c_str(), shuffleSplits,
                     par.compressed,
                     (dbType == -1) ? Parameters::DBTYPE_OMIT_FILE : dbType);
  seqWriter.open();
  size_t headerFileOffset = 0;
  size_t seqFileOffset = 0;

  size_t fileCount = filenames.size();
  DBReader<unsigned int>* reader = NULL;
  if (dbInput == true) {
    reader = new DBReader<unsigned int>(
        out, par.db1.c_str(), par.db1Index.c_str(), 1,
        DBReader<unsigned int>::USE_DATA | DBReader<unsigned int>::USE_INDEX |
            DBReader<unsigned int>::USE_LOOKUP);
    reader->open(DBReader<unsigned int>::LINEAR_ACCCESS);
    fileCount = reader->getSize();
  }

  for (size_t fileIdx = 0; fileIdx < fileCount; fileIdx++) {
    unsigned int numEntriesInCurrFile = 0;
    std::string header;
    header.reserve(1024);

    std::string sourceName;
    if (dbInput == true) {
      unsigned int dbKey = reader->getDbKey(fileIdx);
      size_t lookupId = reader->getLookupIdByKey(dbKey);
      sourceName = reader->getLookupEntryName(lookupId);
    } else {
      sourceName = FileUtil::baseName(out, filenames[fileIdx]);
    }
    char buffer[4096];
    size_t len = snprintf(buffer, sizeof(buffer), "%zu\t%s\n", fileIdx,
                          sourceName.c_str());
    int written = fwrite(buffer, sizeof(char), len, source);
    if (written != (int)len) {
      out->failure("Cannot write to source file {}", sourceFile);
    }

    KSeqWrapper* kseq = NULL;
    if (dbInput == true) {
      kseq = new KSeqBuffer(out, reader->getData(fileIdx, 0),
                            reader->getEntryLen(fileIdx) - 1);
    } else {
      kseq = KSeqFactory(out, filenames[fileIdx].c_str());
    }
    if (par.createdbMode == Parameters::SEQUENCE_SPLIT_MODE_SOFT &&
        kseq->type != KSeqWrapper::KSEQ_FILE) {
      out->warn("Only uncompressed fasta files can be used with --createdb-mode 0. We recompute with --createdb-mode 1.");
      par.createdbMode = Parameters::SEQUENCE_SPLIT_MODE_HARD;
      progress.reset(SIZE_MAX);
      hdrWriter.close();
      seqWriter.close();
      delete kseq;
      if (fclose(source) != 0) {
        out->failure("Cannot close file {}", sourceFile);
      }
      for (size_t i = 0; i < shuffleSplits; ++i) {
        sourceLookup[i].clear();
      }
      goto redoComputation;
    }
    while (kseq->ReadEntry()) {
      progress.updateProgress();
      const KSeqWrapper::KSeqEntry& e = kseq->entry;
      if (e.name.l == 0) {
        out->failure("Fasta entry {} is invalid", entries_num);
      }

      // header
      if (par.createdbMode == Parameters::SEQUENCE_SPLIT_MODE_HARD) {
        header.append(e.name.s, e.name.l);
        if (e.comment.l > 0) {
          header.append(" ", 1);
          header.append(e.comment.s, e.comment.l);
        }

        std::string headerId = Util::parseFastaHeader(header.c_str());
        if (headerId.empty()) {
          // An identifier is necessary for these two cases, so we should just
          // give up
         out->warn("Cannot extract identifier from entry {}", entries_num);
        }
        header.push_back('\n');
      }
      unsigned int id = par.identifierOffset + entries_num;
      if (dbType == -1) {
        // check for the first 10 sequences if they are nucleotide sequences
        if (sampleCount < 10 || (sampleCount % 100) == 0) {
          if (sampleCount < testForNucSequence) {
            size_t cnt = 0;
            for (size_t i = 0; i < e.sequence.l; i++) {
              switch (toupper(e.sequence.s[i])) {
                case 'T':
                case 'A':
                case 'G':
                case 'C':
                case 'U':
                case 'N':
                  cnt++;
                  break;
              }
            }
            const float nuclDNAFraction =
                static_cast<float>(cnt) / static_cast<float>(e.sequence.l);
            if (nuclDNAFraction > 0.9) {
              isNuclCnt += true;
            }
          }
          sampleCount++;
        }
        if (par.createdbMode == Parameters::SEQUENCE_SPLIT_MODE_SOFT &&
            e.multiline == true) {
          out->warn("Multiline fasta can not be combined with --createdb-mode 0. We recompute with --createdb-mode 1");
          par.createdbMode = Parameters::SEQUENCE_SPLIT_MODE_HARD;
          progress.reset(SIZE_MAX);
          hdrWriter.close();
          seqWriter.close();
          delete kseq;
          if (fclose(source) != 0) {
            out->failure("Cannot close file {}", sourceFile);
          }
          for (size_t i = 0; i < shuffleSplits; ++i) {
            sourceLookup[i].clear();
          }
          goto redoComputation;
        }
      }

      // Finally write down the entry
      unsigned int splitIdx = id % shuffleSplits;
      sourceLookup[splitIdx].emplace_back(fileIdx);
      if (par.createdbMode == Parameters::SEQUENCE_SPLIT_MODE_SOFT) {
        // +2 to emulate the \n\0
        hdrWriter.writeIndexEntry(id, headerFileOffset + e.headerOffset,
                                  (e.sequenceOffset - e.headerOffset) + 1, 0);
        seqWriter.writeIndexEntry(id, seqFileOffset + e.sequenceOffset,
                                  e.sequence.l + 2, 0);
      } else {
        hdrWriter.writeData(header.c_str(), header.length(), id, splitIdx);
        seqWriter.writeStart(splitIdx);
        seqWriter.writeAdd(e.sequence.s, e.sequence.l, splitIdx);
        seqWriter.writeAdd(&newline, 1, splitIdx);
        seqWriter.writeEnd(id, splitIdx, true);
      }

      entries_num++;
      numEntriesInCurrFile++;
      header.clear();
    }
    delete kseq;
    if (filenames.size() > 1 &&
        par.createdbMode == Parameters::SEQUENCE_SPLIT_MODE_SOFT) {
      size_t fileSize = FileUtil::getFileSize(out, filenames[fileIdx].c_str());
      headerFileOffset += fileSize;
      seqFileOffset += fileSize;
    }
  }
  out->info("");
  if (fclose(source) != 0) {
    out->failure("Cannot close file {}", sourceFile);
  }
  hdrWriter.close(true, false);
  seqWriter.close(true, false);
  if (dbType == -1) {
    if (isNuclCnt == sampleCount) {
      dbType = Parameters::DBTYPE_NUCLEOTIDES;
    } else {
      dbType = Parameters::DBTYPE_AMINO_ACIDS;
    }
    seqWriter.writeDbtypeFile(out, seqWriter.getDataFileName(), dbType,
                              par.compressed);
  }
  out->info("Database type: {}", Parameters::getDbTypeName(dbType)
                    );
  if (dbInput == true) {
    reader->close();
    delete reader;
  }

  if (entries_num == 0) {
    out->failure("The input files have no entry. Please check your input files. Only files in fasta/fastq[.gz|bz2] are supported");
  }

  // fix ids
  if (par.shuffleDatabase == true) {
    DBWriter::createRenumberedDB(out, dataFile, indexFile, "", "",
                                 DBReader<unsigned int>::LINEAR_ACCCESS);
    DBWriter::createRenumberedDB(out, hdrDataFile, hdrIndexFile, "", "",
                                 DBReader<unsigned int>::LINEAR_ACCCESS);
  }
  if (par.createdbMode == Parameters::SEQUENCE_SPLIT_MODE_SOFT) {
    if (filenames.size() == 1) {
      FileUtil::symlinkAbs(out, filenames[0], dataFile);
      FileUtil::symlinkAbs(out, filenames[0], hdrDataFile);
    } else {
      for (size_t fileIdx = 0; fileIdx < filenames.size(); fileIdx++) {
        FileUtil::symlinkAbs(out, filenames[fileIdx],
                             dataFile + "." + SSTR(fileIdx));
        FileUtil::symlinkAbs(out, filenames[fileIdx],
                             hdrDataFile + "." + SSTR(fileIdx));
      }
    }
  }

  if (par.writeLookup == true) {
    DBReader<unsigned int> readerHeader(
        out, hdrDataFile.c_str(), hdrIndexFile.c_str(), 1,
        DBReader<unsigned int>::USE_DATA | DBReader<unsigned int>::USE_INDEX);
    readerHeader.open(DBReader<unsigned int>::NOSORT);
    // create lookup file
    std::string lookupFile = dataFile + ".lookup";
    FILE* file = FileUtil::openAndDelete(out, lookupFile.c_str(), "w");
    std::string buffer;
    buffer.reserve(2048);
    unsigned int splitIdx = 0;
    unsigned int splitCounter = 0;
    DBReader<unsigned int>::LookupEntry entry;
    for (unsigned int id = 0; id < readerHeader.getSize(); id++) {
      size_t splitSize = sourceLookup[splitIdx].size();
      if (splitSize == 0 || splitCounter > sourceLookup[splitIdx].size() - 1) {
        splitIdx++;
        splitCounter = 0;
      }
      char* header = readerHeader.getData(id, 0);
      entry.id = id;
      entry.entryName = Util::parseFastaHeader(header);
      if (entry.entryName.empty()) {
        out->warn("Cannot extract identifier from entry {}", entries_num);
      }
      entry.fileNumber = sourceLookup[splitIdx][splitCounter];
      readerHeader.lookupEntryToBuffer(buffer, entry);
      int written = fwrite(buffer.c_str(), sizeof(char), buffer.size(), file);
      if (written != (int)buffer.size()) {
        out->failure("Cannot write to lookup file {}", lookupFile);
      }
      buffer.clear();
      splitCounter++;
    }
    if (fclose(file) != 0) {
      out->failure("Cannot close file {}", lookupFile);
    }
    readerHeader.close();
  }
  delete[] sourceLookup;

  return EXIT_SUCCESS;
}
