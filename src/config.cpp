#include "config.hpp"

json fhv::config::loadConfig() {
  std::ifstream i;

  std::string homedir(getenv("HOME"));
  i.open(
    homedir + "/" + configFileLocation_userPostfix + "/" + configFileName);

  if(!i){
    i.open(configFileLocation_system + "/" + configFileName);

    if (!i) {
      std::cerr << "ERROR: no config file exists! Please copy "
        << "\"" << configFileLocation_system << "/" 
        << configFileName_template << "\" "
        << "to \"" << configFileLocation_system << "/"
        << configFileName << "\". "
        << "Then, fill out the file with your machine's performance. "
        << "If you do not have permissions necessary to write "
        << "to \"" << configFileLocation_system << "/"
        << configFileName << "\", "
        << "you may instead create a config file at "
        << "to \"" << homedir << "/" << configFileLocation_userPostfix << "/"
        << configFileName << "\". "
        << "For more information, see \"docs/installation.md\"."
        << std::endl;
      return json();
    }
  }

  json j;
  i >> j;

  return j;
}
