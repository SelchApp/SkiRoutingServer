#pragma once

#include "External/mongoose/mongoose.h"

class Response {

public:

  Response(std::string msg) : res(msg) {}

  void write(mg_connection *c) {
    mg_send_head(c, 200, (int64_t)res.length(), "Content-Type: application/json");
    mg_send(c, res.c_str(), (int)res.length());
  }

private:
  std::string res;
};
