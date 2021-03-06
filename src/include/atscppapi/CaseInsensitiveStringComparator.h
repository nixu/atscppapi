/*
 * Copyright (c) 2013 LinkedIn Corp. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the license at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 */

/**
 * @file CaseInsensitiveStringComparator.h
 * @author Brian Geffon
 * @author Manjesh Nilange
 * @brief A case insensitive comparator that can be used with STL containers.
 */

#pragma once
#ifndef ATSCPPAPI_CASE_INSENSITIVE_STRING_COMPARATOR_H_
#define ATSCPPAPI_CASE_INSENSITIVE_STRING_COMPARATOR_H_

#include <string>

namespace atscppapi {

/**
 * @brief A case insensitive comparator that can be used with standard library containers.
 *
 * The primary use for this class is to make all Headers case insensitive.
 */
class CaseInsensitiveStringComparator {
public:
  /**
   * @return true if lhs is lexicographically "less-than" rhs; meant for use in std::map or other standard library containers.
   */
  bool operator()(const std::string &lhs, const std::string &rhs) const;

  /**
   * @return numerical value of lexicographical comparison a la strcmp
   */
  int compare(const std::string &lhs, const std::string &rhs) const;
};

}

#endif
