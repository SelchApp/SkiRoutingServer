#pragma once

#include <cassert>

#include "DataStructures/Graph/Attributes/AbstractAttribute.h"
#include "Tools/Constants.h"

// An attribute associating a travel cost with each edge of a graph.
class RunTypeAttribute : public AbstractAttribute<char> {
 public:
  static constexpr Type DEFAULT_VALUE = 0;    // The attribute's default value.
  static constexpr const char* NAME = "runtype"; // The attribute's unique name.

  // Returns the travel cost on edge e.
  const Type& runType(const int e) const {
    assert(e >= 0); assert(e < values.size());
    return values[e];
  }

  // Returns a reference to the travel cost on edge e.
  Type& runType(const int e) {
    assert(e >= 0); assert(e < values.size());
    return values[e];
  }
};
