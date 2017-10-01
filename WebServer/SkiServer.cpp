#include "External/routing-framework/Tools/CommandLine/CommandLineParser.h"
#include "External/routing-framework/DataStructures/Graph/Attributes/LengthAttribute.h"
#include "External/routing-framework/DataStructures/Graph/Attributes/LatLngAttribute.h"
#include "External/routing-framework/DataStructures/Graph/Attributes/EdgeIdAttribute.h"
#include "External/routing-framework/DataStructures/Graph/Graph.h"
#include <string>
#include <iostream>
#include <fstream>
#include "WebServer/HttpServer.h"
#include "WebServer/QueryServlet.h"
#include "OsmImport/CodeAttribute.h"
#include "OsmImport/RunTypeAttribute.h"

int main(int argc, char* argv[]) {

  CommandLineParser clp(argc, argv);
  const std::string filename = clp.getValue<std::string>("g");

  using SkiGraph = StaticGraph<VertexAttrs<LatLngAttribute>, EdgeAttrs<LengthAttribute, CodeAttribute, EdgeIdAttribute, RunTypeAttribute>>;
  std::ifstream in(filename, std::ios::binary);
  SkiGraph graph(in);

  int i = 0;
  FORALL_EDGES(graph, e)
    graph.edgeId(e) = i++;

  QueryServlet serv(graph);

  HttpServer skiServer;
  skiServer.registerServlet(&serv);

  skiServer.start();

}
