#include <biosnake/commons/commandCaller.h>
#include <biosnake/output.h>
#include <biosnake/commons/fileUtil.h>
#include <biosnake/commons/parameters.h>
#include <biosnake/commons/util.h>
#include <biosnake/output.h>
#include "rbh.sh.h"

#include <cassert>

void setRbhDefaults(Parameters *p) {
  p->compBiasCorrection = 0;
  p->alignmentMode = Parameters::ALIGNMENT_MODE_SCORE_COV_SEQID;
  p->maskMode = 0;
  p->orfStartMode = 1;
  p->orfMinLength = 10;
  p->orfMaxLength = 32734;
}

int rbh(biosnake_output *out, Parameters &par) {
  //    Parameters &par = Parameters::getInstance();
  //    setRbhDefaults(&par);
  //
  //    // set a lot of possibly misleading comments to EXPERT mode
  //    par.PARAM_OVERLAP.addCategory(BiosnakeParameter::COMMAND_EXPERT);
  //    par.PARAM_DB_OUTPUT.addCategory(BiosnakeParameter::COMMAND_EXPERT);
  //
  //    for (size_t i = 0; i < par.extractorfs.size(); i++){
  //        par.extractorfs[i]->addCategory(BiosnakeParameter::COMMAND_EXPERT);
  //    }
  //    for (size_t i = 0; i < par.translatenucs.size(); i++){
  //        par.translatenucs[i]->addCategory(BiosnakeParameter::COMMAND_EXPERT);
  //    }
  //    for (size_t i = 0; i < par.splitsequence.size(); i++) {
  //        par.splitsequence[i]->addCategory(BiosnakeParameter::COMMAND_EXPERT);
  //    }
  //    // restore threads and verbosity
  //    par.PARAM_COMPRESSED.removeCategory(BiosnakeParameter::COMMAND_EXPERT);
  //    par.PARAM_V.removeCategory(BiosnakeParameter::COMMAND_EXPERT);
  //    par.PARAM_THREADS.removeCategory(BiosnakeParameter::COMMAND_EXPERT);
  //
  //    par.parseParameters(argc, argv, command, true, 0, 0);

  std::string tmpDir = par.db4;
  std::string hash = SSTR(par.hashParameter(out, par.databases_types, par.filenames,
                                            par.searchworkflow));
  if (par.reuseLatest) {
    hash = FileUtil::getHashFromSymLink(out, tmpDir + "/latest");
  }
  tmpDir = FileUtil::createTemporaryDirectory(out, par.baseTmpPath, tmpDir, hash);
  par.filenames.pop_back();
  par.filenames.push_back(tmpDir);

  CommandCaller cmd(out);
  cmd.addVariable("SEARCH_A_B_PAR",
                  par.createParameterString(out, par.searchworkflow).c_str());
  int originalCovMode = par.covMode;
  par.covMode = Util::swapCoverageMode(out, par.covMode);
  cmd.addVariable("SEARCH_B_A_PAR",
                  par.createParameterString(out, par.searchworkflow).c_str());
  par.covMode = originalCovMode;
  cmd.addVariable("REMOVE_TMP", par.removeTmpFiles ? "TRUE" : NULL);
  cmd.addVariable("VERB_COMP_PAR",
                  par.createParameterString(out, par.verbandcompression).c_str());
  cmd.addVariable("THREADS_COMP_PAR",
                  par.createParameterString(out, par.threadsandcompression).c_str());
  cmd.addVariable("VERBOSITY",
                  par.createParameterString(out, par.onlyverbosity).c_str());
  std::string program = tmpDir + "/rbh.sh";
  FileUtil::writeFile(out, program, rbh_sh, rbh_sh_len);
  cmd.execProgram(program.c_str(), par.filenames);

  // Should never get here
  assert(false);
  return 0;
}
