#include "External/routing-framework/DataStructures/Graph/Graph.h"
#include "External/routing-framework/DataStructures/Graph/Attributes/LatLngAttribute.h"
#include "External/routing-framework/DataStructures/Graph/Attributes/LengthAttribute.h"
#include "External/routing-framework/DataStructures/Graph/Attributes/CoordinateAttribute.h"
#include "External/routing-framework/Algorithms/GraphTraversal/StronglyConnectedComponents.h"
#include "External/Visualization/Graphics/PngGraphic.h"
#include "External/Visualization/GraphDrawer.h"
#include "CodeAttribute.h"
#include "RunTypeAttribute.h"

#include <routingkit/tag_map.h>
#include <routingkit/osm_graph_builder.h>
#include <routingkit/osm_profile.h>

#include <string>
#include <functional>
#include <cstdint>

using SkiGraph = StaticGraph<VertexAttrs<LatLngAttribute, CoordinateAttribute>, EdgeAttrs<LengthAttribute, CodeAttribute, RunTypeAttribute>>;

bool str_eq(const char*l, const char*r){
  return !strcmp(l, r);
}

bool is_osm_way_used_by_ski(uint64_t osm_way_id, const RoutingKit::TagMap&tags, std::function<void(const std::string&)>log_message){
//  const char* ref = tags["ref"];
//  if(ref == nullptr)
//      return false;
//
  const char* area = tags["area"];
  if(area != nullptr && str_eq(area, "yes"))
      return false;

  const char* diff = tags["piste:difficulty"];
  if(diff != nullptr && str_eq(diff, "freeride"))
      return false;
//
//  const char* groom = tags["piste:grooming"];
//  if(groom != nullptr && str_eq(groom, "backcountry"))
//      return false;

    const char* aerialway = tags["aerialway"];
    if(aerialway != nullptr)
        return true;

    const char* route = tags["piste:type"];
    if(route && str_eq(route, "downhill"))
        return true;

    return false;
}

RoutingKit::OSMWayDirectionCategory get_osm_ski_direction_category(uint64_t osm_way_id, const RoutingKit::TagMap&tags, std::function<void(const std::string&)>log_message){
    const char *oneway = tags["oneway"];
    if(oneway != nullptr){
        if(str_eq(oneway, "-1") || str_eq(oneway, "reverse") || str_eq(oneway, "backward")) {
            return RoutingKit::OSMWayDirectionCategory::only_open_backwards;
        } else if(str_eq(oneway, "yes") || str_eq(oneway, "true") || str_eq(oneway, "1")) {
            return RoutingKit::OSMWayDirectionCategory::only_open_forwards;
        } else if(str_eq(oneway, "no") || str_eq(oneway, "false") || str_eq(oneway, "0")) {
            return RoutingKit::OSMWayDirectionCategory::open_in_both;
        } else if(str_eq(oneway, "reversible") || str_eq(oneway, "alternating")) {
            return RoutingKit::OSMWayDirectionCategory::closed;
        } else {
            log_message("Warning: OSM way "+std::to_string(osm_way_id)+" has unknown oneway tag value \""+oneway+"\" for \"oneway\". Way is closed.");
        }
    }
    return RoutingKit::OSMWayDirectionCategory::open_in_both;
}

std::string get_osm_way_ref(uint64_t osm_way_id, const RoutingKit::TagMap&tags, std::function<void(const std::string&)>log_message){
    auto ref = tags["ref"];

    if(ref != nullptr)
        return std::string(ref);
    else
        return std::string();
}

char get_osm_way_type(uint64_t osm_way_id, const RoutingKit::TagMap&tags, std::function<void(const std::string&)>log_message){
  const char* aerialway = tags["aerialway"];
  if(aerialway != nullptr)
      return 0;
    else
        return 1;
}


SkiGraph simple_load_osm_ski_routing_graph_from_pbf(
    const std::string&pbf_file,
    const std::function<void(const std::string&)>&log_message = nullptr,
    bool all_modelling_nodes_are_routing_nodes = false,
    bool file_is_ordered_even_though_file_header_says_that_it_is_unordered = false
){

    auto mapping = RoutingKit::load_osm_id_mapping_from_pbf(
        pbf_file,
        nullptr,
        [&](uint64_t osm_way_id, const RoutingKit::TagMap&tags){
            return is_osm_way_used_by_ski(osm_way_id, tags, log_message);
        },
        log_message
        ,all_modelling_nodes_are_routing_nodes
    );

    unsigned routing_way_count = mapping.is_routing_way.population_count();

    std::vector<std::string> run_or_lift_code(routing_way_count);
    std::vector<std::string> run_or_lift_name(routing_way_count);
    std::vector<char> runType(routing_way_count);

    auto routing_graph = load_osm_routing_graph_from_pbf(
        pbf_file,
        mapping,
        [&](uint64_t osm_way_id, unsigned routing_way_id, const RoutingKit::TagMap&way_tags){
            run_or_lift_code[routing_way_id] = get_osm_way_ref(osm_way_id, way_tags, log_message);
            run_or_lift_name[routing_way_id] = RoutingKit::get_osm_way_name(osm_way_id, way_tags, log_message);
            runType[routing_way_id] = get_osm_way_type(osm_way_id, way_tags, log_message);
            return get_osm_ski_direction_category(osm_way_id, way_tags, log_message);
        },
        nullptr,
        log_message
    );

    unsigned arc_count = routing_graph.head.size();

    mapping = RoutingKit::OSMRoutingIDMapping(); // release memory

    SkiGraph dst;
    for (int v = 0; v != routing_graph.latitude.size(); ++v) {
      dst.appendVertex();
      dst.latLng(v) = {routing_graph.latitude[v], routing_graph.longitude[v]};
      for (int e = routing_graph.first_out[v]; e != routing_graph.first_out[v + 1]; ++e) {
        dst.appendEdge(routing_graph.head[e]);
        dst.runType(e) = runType[routing_graph.way[e]];
        dst.template get<LengthAttribute>(e) = routing_graph.geo_distance[e];
        std::string codeStr = run_or_lift_code[routing_graph.way[e]];
        for (int i = 0; i <= codeStr.size(); ++i)
          dst.code(e)[i] = codeStr.c_str()[i];
      }
    }


//    ret.run_or_lift_code.resize(arc_count);
//    ret.run_or_lift_name.resize(arc_count);
//    for(unsigned a=0; a<arc_count; ++a) {
//        ret.run_or_lift_code[a] = run_or_lift_code[routing_graph.way[a]];
//        ret.run_or_lift_name[a] = run_or_lift_name[routing_graph.way[a]];
//    }

    return dst;
}


int main() {
  SkiGraph graph = simple_load_osm_ski_routing_graph_from_pbf("/home/valentin/Innohacks/kaiser.pbf", nullptr, true);
  StronglyConnectedComponents scc;
  scc.run(graph);
  graph.extractVertexInducedSubgraph(scc.getLargestSccAsBitmask());

  Rectangle rect = Rectangle{Point{graph.latLng(0).webMercatorProjection()}};
  FORALL_VERTICES(graph, v) {
    graph.coordinate(v) = graph.latLng(v).webMercatorProjection();
    rect.extend(graph.coordinate(v));
  }

  PngGraphic graphic("/home/valentin/Innohacks/kaiser", 14, 14, rect);
  GraphDrawer gd(&graphic, [&graph](const int v) { return graph.coordinate(v);});
  gd.drawGraph(graph);

  std::ofstream out("/home/valentin/Innohacks/kaiser.gr.bin", std::ios::binary);
  graph.writeTo(out);

}
