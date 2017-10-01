#pragma once

#include <cassert>

#include "DataStructures/Graph/Attributes/AbstractAttribute.h"
#include "Tools/Constants.h"

// An attribute associating a travel cost with each edge of a graph.
class CodeAttribute : public AbstractAttribute<std::array<char, 255>> {
 public:
  static constexpr Type DEFAULT_VALUE = { 'd' };       // The attribute's default value.
  static constexpr const char* NAME = "code"; // The attribute's unique name.

  // Returns the travel cost on edge e.
  const Type& code(const int e) const {
    assert(e >= 0); assert(e < values.size());
    return values[e];
  }

  // Returns a reference to the travel cost on edge e.
  Type& code(const int e) {
    assert(e >= 0); assert(e < values.size());
    return values[e];
  }
};

constexpr CodeAttribute::Type CodeAttribute::DEFAULT_VALUE;
