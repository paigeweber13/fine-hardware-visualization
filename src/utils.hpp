//
// Created by riley on 10/20/20.
//

#pragma once

#include <iostream>
#include <string>

namespace fhv {
  namespace utils {
    /*
     * used to create directories. For example, if "foo/bar/x" is supplied, will
     * create directories "foo/bar". If they already exist, this will do nothing
     * silently
     */
    int create_directories_for_file(std::string file);
  };
};
