#pragma once

#include "WebServer/Servlet.h"
#include <vector>
#include "External/mongoose/mongoose.h"

class HttpServer {

 public:
  HttpServer() {
    current = this; // absurd
  }

  void start() {
    mg_mgr_init(&mgr, nullptr);
    c = mg_bind(&mgr, "15235", callback);
    mg_set_protocol_http_websocket(c);

    for (;;) {
      mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
  }

  void registerServlet(Servlet* serv) {
    servlets.push_back(serv);
  }


 private:

  static void callback(mg_connection *c, int ev, void *p) {
    if (ev == MG_EV_HTTP_REQUEST) {
      http_message *hm = (http_message *) p;

      // We have received an HTTP request. Parsed request is contained in `hm`.

      current->processRequest(c, hm);
    }
  }

  void processRequest(mg_connection *c, http_message *p) {
    std::string uri(p->uri.p+1, p->uri.len-1);
    for (Servlet* serv : servlets) {
      if (serv->handler() == uri)
        serv->process(c, p);
    }
  }

  mg_mgr mgr;
  mg_connection *c = nullptr;
  std::vector<Servlet*> servlets;
  static HttpServer *current;


};

HttpServer* HttpServer::current;
