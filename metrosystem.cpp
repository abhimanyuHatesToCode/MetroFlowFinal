#include "metrosystem.h"
#include <fstream>
#include <sstream>
#include <queue>
#include <iostream>
#include <iomanip>
#include <QDebug>   // For Qt style debugging output
#include <algorithm> // For std::sort

// Utility function implementation
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

MetroSystem::MetroSystem() {}

// DEBUGGING VERSION of loadMetroData for 10-column CSV
bool MetroSystem::loadMetroData(const std::string& filename, std::string& errorMsg) {
    qDebug() << "MetroSystem::loadMetroData called for file:" << QString::fromStdString(filename);
    std::ifstream file(filename);
    if (!file.is_open()) {
        errorMsg = "Failed to open file: " + filename;
        qCritical() << "LOAD_DATA_ERROR:" << QString::fromStdString(errorMsg);
        return false;
    }

    graph_.clear();
    stationNames_.clear();
    stationCoordinates_.clear();
    std::string lineStr;

    std::string headerLine;
    if (!getline(file, headerLine)) {
        errorMsg = "File is empty or failed to read header: " + filename;
        qCritical() << "LOAD_DATA_ERROR:" << QString::fromStdString(errorMsg);
        file.close();
        return false;
    }
    qDebug() << "CSV Header:" << QString::fromStdString(headerLine);

    int lineNumber = 1;
    int successfullyParsedRows = 0;

    while (getline(file, lineStr)) {
        lineNumber++;
        if (trim(lineStr).empty()) {
            continue;
        }
        qDebug().noquote() << "\n--- Processing CSV Line" << lineNumber << "---";
        qDebug().noquote() << "Raw Line Content: " << QString::fromStdString(lineStr);

        std::stringstream ss(lineStr);
        std::string fromStation_csv, toStation_csv, timeStr_csv, distStr_csv, costStr_csv,
            segmentLine_csv, // This is your 6th column "Line"
            latFromStr_csv, lonFromStr_csv, latToStr_csv, lonToStr_csv;

        bool readSuccess = true;
        if (!getline(ss, fromStation_csv, ',')) readSuccess = false;
        if (readSuccess && !getline(ss, toStation_csv, ',')) readSuccess = false;
        if (readSuccess && !getline(ss, timeStr_csv, ',')) readSuccess = false;
        if (readSuccess && !getline(ss, distStr_csv, ',')) readSuccess = false;
        if (readSuccess && !getline(ss, costStr_csv, ',')) readSuccess = false;
        if (readSuccess && !getline(ss, segmentLine_csv, ',')) readSuccess = false; // 6th column
        if (readSuccess && !getline(ss, latFromStr_csv, ',')) readSuccess = false;
        if (readSuccess && !getline(ss, lonFromStr_csv, ',')) readSuccess = false;
        if (readSuccess && !getline(ss, latToStr_csv, ',')) readSuccess = false;
        if (readSuccess && !getline(ss, lonToStr_csv)) readSuccess = false;      // 10th column, read till end

        if (!readSuccess) {
            qWarning().noquote() << "PARSING_WARNING (Line " << lineNumber << "): Failed to read all 10 expected fields. Line might have too few columns or be malformed. Skipping line.";
            qDebug().noquote() << "  Partial Raw Fields: FS=" << QString::fromStdString(fromStation_csv) << " TS=" << QString::fromStdString(toStation_csv) << "...";
            continue;
        }

        qDebug().noquote() << "  Raw Fields: FS=" << QString::fromStdString(fromStation_csv)
                           << " TS=" << QString::fromStdString(toStation_csv)
                           << " Time=" << QString::fromStdString(timeStr_csv)
                           << " Dist=" << QString::fromStdString(distStr_csv)
                           << " Cost=" << QString::fromStdString(costStr_csv)
                           << " SegLine=" << QString::fromStdString(segmentLine_csv) // Changed name for clarity
                           << " LatF=" << QString::fromStdString(latFromStr_csv)
                           << " LonF=" << QString::fromStdString(lonFromStr_csv)
                           << " LatT=" << QString::fromStdString(latToStr_csv)
                           << " LonT=" << QString::fromStdString(lonToStr_csv);

        std::string fromStation = trim(fromStation_csv);
        std::string toStation = trim(toStation_csv);
        std::string timeStr = trim(timeStr_csv);
        std::string distStr = trim(distStr_csv);
        std::string costStr = trim(costStr_csv);
        std::string segmentLine = trim(segmentLine_csv); // This is the crucial line name
        std::string latFromStr = trim(latFromStr_csv);
        std::string lonFromStr = trim(lonFromStr_csv);
        std::string latToStr = trim(latToStr_csv);
        std::string lonToStr = trim(lonToStr_csv);

        qDebug().noquote() << "  Trimmed Fields: FS=" << QString::fromStdString(fromStation)
                           << " TS=" << QString::fromStdString(toStation)
                           << " Time=" << QString::fromStdString(timeStr)
                           << " SegLine=" << QString::fromStdString(segmentLine)
                           << " LatF=" << QString::fromStdString(latFromStr)
                           << " LonF=" << QString::fromStdString(lonFromStr)
                           << " LatT=" << QString::fromStdString(latToStr)
                           << " LonT=" << QString::fromStdString(lonToStr);

        if (fromStation.empty() || toStation.empty() || timeStr.empty() || distStr.empty() || costStr.empty() ||
            segmentLine.empty() || latFromStr.empty() || lonFromStr.empty() || latToStr.empty() || lonToStr.empty()) {
            qWarning().noquote() << "PARSING_WARNING (Line " << lineNumber << "): Skipping line due to one or more critical fields being empty after trimming.";
            continue;
        }

        try {
            int timeVal = std::stoi(timeStr);
            double distanceVal = std::stod(distStr);
            int costVal = std::stoi(costStr);
            double latFrom = std::stod(latFromStr);
            double lonFrom = std::stod(lonFromStr);
            double latTo = std::stod(latToStr);
            double lonTo = std::stod(lonToStr);

            qDebug().noquote() << "    Converted Numerics: Time=" << timeVal << " Cost=" << costVal
                               << " LatF=" << latFrom << " LonF=" << lonFrom << " LatT=" << latTo << " LonT=" << lonTo;

            if (stationCoordinates_.find(fromStation) == stationCoordinates_.end()) {
                stationCoordinates_[fromStation] = QPointF(lonFrom, latFrom);
            }
            if (stationCoordinates_.find(toStation) == stationCoordinates_.end()) {
                stationCoordinates_[toStation] = QPointF(lonTo, latTo);
            }

            graph_[fromStation].push_back(Edge(toStation, timeVal, distanceVal, costVal, segmentLine));
            graph_[toStation].push_back(Edge(fromStation, timeVal, distanceVal, costVal, segmentLine));

            stationNames_.insert(fromStation);
            stationNames_.insert(toStation);
            successfullyParsedRows++;

        } catch (const std::invalid_argument& e) {
            qWarning().noquote() << "PARSING_ERROR (Line " << lineNumber << "): Invalid numeric argument for a field: " << e.what() << ". Skipping line.";
        } catch (const std::out_of_range& e) {
            qWarning().noquote() << "PARSING_ERROR (Line " << lineNumber << "): Out of range numeric value for a field: " << e.what() << ". Skipping line.";
        }
    }
    file.close();
    qDebug() << "Finished parsing file. Total data lines processed:" << (lineNumber -1);
    qDebug() << "Successfully parsed rows into graph:" << successfullyParsedRows;

    if (graph_.empty() && (lineNumber-1 > 0 && successfullyParsedRows == 0) ) {
        errorMsg = "No data loaded from file or file format incorrect after parsing: " + filename;
        qCritical() << "LOAD_DATA_FINAL_ERROR:" << QString::fromStdString(errorMsg);
        qCritical() << "All " << (lineNumber-1) << " data lines were skipped or failed parsing. Check warnings above.";
        return false;
    }
    if (graph_.empty() && (lineNumber-1 == 0)){
        errorMsg = "No data lines found in file after header: " + filename;
        qCritical() << "LOAD_DATA_FINAL_ERROR:" << QString::fromStdString(errorMsg);
        return false;
    }

    qInfo() << "Metro data loaded successfully. " << stationNames_.size() << " unique stations found.";
    qInfo() << stationCoordinates_.size() << " station coordinates stored.";
    errorMsg = "";
    return true;
}

const std::unordered_map<std::string, QPointF>& MetroSystem::getStationCoordinates() const {
    return stationCoordinates_;
}

std::vector<std::string> MetroSystem::getStationNames() const {
    std::vector<std::string> names(stationNames_.begin(), stationNames_.end());
    std::sort(names.begin(), names.end());
    return names;
}

const Edge* MetroSystem::findEdge(const std::string& from, const std::string& to) const {
    auto it = graph_.find(from);
    if (it != graph_.end()) {
        for (const auto& edge : it->second) {
            if (edge.to == to) {
                return &edge;
            }
        }
    }
    return nullptr;
}

std::vector<PathSegment> MetroSystem::findPathLeastStops(const std::string& start, const std::string& end) {
    if (graph_.find(start) == graph_.end() || graph_.find(end) == graph_.end()) return {};
    if (start == end) return {PathSegment(start, "", 0, 0, true)};

    std::unordered_map<std::string, std::string> parentNode;
    std::unordered_map<std::string, bool> visited;
    std::queue<std::string> q;

    visited[start] = true;
    q.push(start);
    bool found = false;

    while (!q.empty()) {
        std::string curr = q.front(); q.pop();
        if (curr == end) {
            found = true;
            break;
        }
        auto it = graph_.find(curr);
        if (it != graph_.end()) {
            for (const auto& edge : it->second) {
                if (visited.find(edge.to) == visited.end()) {
                    visited[edge.to] = true;
                    parentNode[edge.to] = curr;
                    q.push(edge.to);
                }
            }
        }
    }

    std::vector<PathSegment> path;
    if (found) {
        std::string currentStation = end;
        std::string prevStation;
        while (currentStation != start) {
            if (parentNode.find(currentStation) == parentNode.end()) {
                qCritical() << "BFS Path reconstruction broken for:" << QString::fromStdString(currentStation);
                return {};
            }
            prevStation = parentNode[currentStation];
            const Edge* e = findEdge(prevStation, currentStation);
            if (!e) {
                qCritical() << "Critical error: Edge not found during BFS path reconstruction from" << QString::fromStdString(prevStation) << "to" << QString::fromStdString(currentStation);
                return {};
            }
            path.push_back(PathSegment(currentStation, e->line, e->time, e->cost));
            currentStation = prevStation;
        }
        path.push_back(PathSegment(start, "", 0, 0, true));
        std::reverse(path.begin(), path.end());
    }
    return path;
}

std::vector<PathSegment> MetroSystem::dijkstra(const std::string& start, const std::string& end, const std::string& criteria) {
    if (graph_.find(start) == graph_.end() || graph_.find(end) == graph_.end()) return {};
    if (start == end) return {PathSegment(start, "", 0, 0, true)};

    std::unordered_map<std::string, long long> accumulatedValue;
    std::unordered_map<std::string, std::string> parentNode;

    auto cmp = [&](const std::pair<long long, std::string>& a, const std::pair<long long, std::string>& b) {
        return a.first > b.first;
    };
    std::priority_queue<std::pair<long long, std::string>, std::vector<std::pair<long long, std::string>>, decltype(cmp)> pq(cmp);

    for (const auto& stationName : stationNames_) {
        accumulatedValue[stationName] = std::numeric_limits<long long>::max();
    }
    accumulatedValue[start] = 0;
    pq.push({0, start});
    bool found = false;

    while (!pq.empty()) {
        auto top = pq.top(); pq.pop();
        long long currentAccVal = top.first;
        std::string currNode = top.second;

        if (accumulatedValue.count(currNode) && currentAccVal > accumulatedValue[currNode]) {
            continue;
        }
        if (currNode == end) {
            found = true;
            break;
        }

        auto it = graph_.find(currNode);
        if (it != graph_.end()) {
            for (const auto& edge : it->second) {
                int weight = (criteria == "time") ? edge.time : edge.cost;
                if (accumulatedValue[currNode] != std::numeric_limits<long long>::max() &&
                    (!accumulatedValue.count(edge.to) || accumulatedValue[currNode] + weight < accumulatedValue[edge.to])) {
                    accumulatedValue[edge.to] = accumulatedValue[currNode] + weight;
                    parentNode[edge.to] = currNode;
                    pq.push({accumulatedValue[edge.to], edge.to});
                }
            }
        }
    }

    std::vector<PathSegment> path;
    if (found) {
        std::string currentStation = end;
        std::string prevStation;
        while (currentStation != start) {
            if (parentNode.find(currentStation) == parentNode.end()) {
                qCritical() << "Dijkstra Path reconstruction broken for:" << QString::fromStdString(currentStation);
                return {};
            }
            prevStation = parentNode[currentStation];
            const Edge* e = findEdge(prevStation, currentStation);
            if (!e) {
                qCritical() << "Critical error: Edge not found during Dijkstra path reconstruction from" << QString::fromStdString(prevStation) << "to" << QString::fromStdString(currentStation);
                return {};
            }
            path.push_back(PathSegment(currentStation, e->line, e->time, e->cost));
            currentStation = prevStation;
        }
        path.push_back(PathSegment(start, "", 0, 0, true));
        std::reverse(path.begin(), path.end());
    }
    return path;
}

std::vector<PathSegment> MetroSystem::findPathByTime(const std::string& start, const std::string& end) {
    return dijkstra(start, end, "time");
}

std::vector<PathSegment> MetroSystem::findPathByCost(const std::string& start, const std::string& end) {
    return dijkstra(start, end, "cost");
}
