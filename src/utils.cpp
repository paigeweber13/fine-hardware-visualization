//
// Created by riley on 10/20/20.
//

#include "utils.hpp"

void fhv::utils::create_directories_for_file(std::string file) {
  size_t pos = 0;
  while(file.find("/", pos+1) != std::string::npos){
    pos = file.find("/", pos+1);
  }
  std::string dir = file.substr(0, pos);

  if(system(("mkdir -p " + dir).c_str()) != 0)
    std::cout << "WARN: there was a problem making the directory for "
              << "color swatches (" << dir << ")." << std::endl
              << "Swatch creation will likely fail.";
}

