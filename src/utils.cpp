//
// Created by riley on 10/20/20.
//

#include "utils.hpp"

/*
 * returns the exit code of the mkdir command.
 */
int fhv::utils::create_directories_for_file(std::string file) {
  size_t pos = 0;
  while(file.find("/", pos+1) != std::string::npos){
    pos = file.find("/", pos+1);
  }
  std::string dir = file.substr(0, pos);

  if (dir == "") {
    // nothing to do, base dir is this directory
    return 0;
  }

  int returnCode = system(("mkdir -p " + dir).c_str());
  if(returnCode != 0)
    std::cerr << "WARN: there was a problem making the directory (" << dir 
              << ")." << std::endl;
  
  return returnCode;
}

