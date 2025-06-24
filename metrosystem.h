#ifndef METROSYSTEM_H
#define METROSYSTEM_H

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <limits>
#include <algorithm>
#include <utility> // For std::move
// #include <tuple>   // Not needed if getAllUniqueEdges is removed

#include <QPointF> // For storing geographic coordinates

// PathSegment Struct Definition
struct PathSegment {
    std::string stationName;
    std::string lineTakenToReach;
    int timeForSegment;
    int costForSegment;
    bool isFirstSegment = false;

    PathSegment(std::string name, std::string line = "", int time = 0, int cost = 0, bool first = false)
        : stationName(std::move(name)), lineTakenToReach(std::move(line)),
        timeForSegment(time), costForSegment(cost), isFirstSegment(first) {}
};

// Edge Struct Definition
struct Edge {
    std::string to;
    int time;
    double distance; // Still present from CSV, though not primary for pathfinding types here
    int cost;
    std::string line; // This should be the line of the track segment

    Edge(const std::string& t, int ti, double d, int c, const std::string& l)
        : to(t), time(ti), distance(d), cost(c), line(l) {}
};

// Utility function
std::string trim(const std::string& str);

class MetroSystem {
public:
    MetroSystem();

    bool loadMetroData(const std::string& filename, std::string& errorMsg);
    std::vector<std::string> getStationNames() const;
    const std::unordered_map<std::string, QPointF>& getStationCoordinates() const;

    // Pathfinding methods remain the same
    std::vector<PathSegment> findPathLeastStops(const std::string& start, const std::string& end);
    std::vector<PathSegment> findPathByTime(const std::string& start, const std::string& end);
    std::vector<PathSegment> findPathByCost(const std::string& start, const std::string& end);

private:
    std::unordered_map<std::string, std::vector<Edge>> graph_;
    std::set<std::string> stationNames_;
    std::unordered_map<std::string, QPointF> stationCoordinates_; // To store station coordinates

    const Edge* findEdge(const std::string& from, const std::string& to) const;
    std::vector<PathSegment> dijkstra(const std::string& start, const std::string& end, const std::string& criteria);
};

#endif // METROSYSTEM_H
