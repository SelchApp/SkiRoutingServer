#pragma once

#include "External/mongoose/mongoose.h"
#include <string>

class Servlet {
public:
  Servlet(std::string uri) : uri(uri) {}
  virtual ~Servlet() {}

  std::string handler() const { return uri; };

  virtual void process(mg_connection *c, http_message *p) = 0;


private:
  std::string uri;
};
