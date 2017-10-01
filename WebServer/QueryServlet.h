#pragma once

#include "External/routing-framework/DataStructures/Graph/Attributes/LatLngAttribute.h"
#include "External/routing-framework/DataStructures/Graph/Attributes/LengthAttribute.h"
#include "External/routing-framework/DataStructures/Graph/Attributes/EdgeIdAttribute.h"
#include "External/routing-framework/DataStructures/Graph/Graph.h"
#include "External/routing-framework/Algorithms/Dijkstra/Dijkstra.h"
#include "External/routing-framework/DataStructures/Labels/BasicLabelSet.h"
#include "External/routing-framework/DataStructures/Geometry/PointSet.h"
#include "WebServer/Servlet.h"
#include "WebServer/Response.h"
#include "OsmImport/CodeAttribute.h"
#include "OsmImport/RunTypeAttribute.h"
#include "External/routing-framework/DataStructures/Geometry/KdTree/StaticKdTree.h"

class QueryServlet : public Servlet {

  using SkiGraph = StaticGraph<VertexAttrs<LatLngAttribute>, EdgeAttrs<LengthAttribute, CodeAttribute, EdgeIdAttribute, RunTypeAttribute>>;
  using Dijkstra = StandardDijkstra<SkiGraph, LengthAttribute, BasicLabelSet<0, ParentInfo::FULL_PARENT_INFO>>;

public:
  QueryServlet(const SkiGraph& graph) : Servlet("query"), graph(graph), dij(graph), psa(ps), kdtree(psa) {

    FORALL_VERTICES(graph, v) {
      ps.insert(graph.latLng(v).webMercatorProjection());
    }
    kdtree.rebuild();
  }


  virtual void process(mg_connection *c, http_message *p) {

    const double slat = lexicalCast<double>(getVariable(p, "slat"));
    const double slng = lexicalCast<double>(getVariable(p, "slng"));
    const double tlat = lexicalCast<double>(getVariable(p, "tlat"));
    const double tlng = lexicalCast<double>(getVariable(p, "tlng"));

    LatLng src(slat, slng);
    LatLng tar(tlat, tlng);

    int srcId, tarId;
    kdtree::EuclideanMetric<kdtree::PointSetAdapter>::DistanceType dist;
    kdtree.nearestNeighborQuery(src.webMercatorProjection(), 1, &srcId, &dist);
    kdtree.nearestNeighborQuery(tar.webMercatorProjection(), 1, &tarId, &dist);



    const int s = srcId;//lexicalCast<int>(getVariable(p, "src"));
    const int t = tarId; //lexicalCast<int>(getVariable(p, "tar"));

    dij.run(s, t);


    std::string res("{\n  \"steps\": [\n\n");
    std::vector<int> path = dij.getReversePath(t);
    std::vector<int> epath = dij.getReverseEdgePath(t);
    std::reverse(path.begin(), path.end());
    std::reverse(epath.begin(), epath.end());
    epath.push_back(epath.back());

    for (int i = 0; i < path.size(); ++i) {
      int first = i;
      std::string currentCode = std::string(&graph.code(epath[i]).front());
      char type = graph.runType(epath[i]);


      std::string instr;
      if (type == 0) {
        instr = "Take lift number " + currentCode;
      } else {
        if (!currentCode.empty())
          instr = "Take ski run number " + currentCode;
        else
          instr = "Take connecting piste";
      }


      res += (i == 0 ? "" : ",");
      res += " { \"instructions\": \""+ instr + "\", \"type\": \"" + std::string(type == 0 ? "lift" : "piste") + "\"" +
          ", \"id\": \"" + currentCode + "\", \"path\": [";

      while (i < path.size() && currentCode == std::string(&graph.code(epath[i]).front())) {
        res += (i == first ? "" : ",");
        res += "{ \"lat\": " + std::to_string(graph.latLng(path[i]).latInDeg()) + ", \"lng\": "+ std::to_string(graph.latLng(path[i]).lngInDeg()) + "}";
        ++i;
      }
      if (i != path.size())
        res += ",{ \"lat\": " + std::to_string(graph.latLng(path[i]).latInDeg()) + ", \"lng\": "+ std::to_string(graph.latLng(path[i]).lngInDeg()) + "}";

      --i;

      res += "]}";



//      res += "        { \"lat\": " + std::to_string(graph.latLng(path[i]).latInDeg()) +
//                       ", \"lng\": "+ std::to_string(graph.latLng(path[i]).lngInDeg()) +
//                       ", \"code\": \"" + std::string(&graph.code(epath[i]).front()) + "\"" +
//                       ", \"type\": \"" + std::to_string(graph.runType(epath[i])) + "\"" +" }" + (i != path.size()-1 ? "," : "") + "\n";
    }

    res += "      ]\n    }\n";

    Response(res).write(c);
  }

  std::string getVariable(http_message *p, const std::string& name) {
    char buf[2550];
    mg_get_http_var(&(p->query_string), name.c_str(), buf, 2550);
    return std::string(buf);
  }



private:

  PointSet ps;
  kdtree::PointSetAdapter psa;
  StaticKdTree<> kdtree;

  const SkiGraph& graph;
  Dijkstra dij;
};
