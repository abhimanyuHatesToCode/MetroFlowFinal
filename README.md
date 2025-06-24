Hello Dear users, I am Abhimanyu Gusain and I have made this project with my team, Harshita Mehta and Prableen Kaur. Without them I won't be able to make it happen and can't thank them enough :(

There are some prerequisites :
* Qt framework (and that too Qt 6)
* Tkinter and Folium installed on the device
* CMake version 3.10 or higher (ideally 3.16+ for modern Qt integration).
* Any code editor ( I have used an IDE CLion )
* Python and C++ installed on your device


So this is the workflow how the code works, we have two main parts: the C++ Qt application and the Python script.

**Part 1: The C++ Qt Application (`MainWindow`, `MetroSystem`)**

**A. Initialization and Data Loading (`MainWindow::loadData`, `MetroSystem::loadMetroData`)**

1.  **`main.cpp` starts `QApplication` and creates `MainWindow`.**
2.  **`MainWindow` Constructor:**
    *   Calls `setupUi()` to create all the GUI elements (ComboBoxes, Button, `QTextEdit` for textual output).
    *   Calls `loadData()`.
3.  **`MainWindow::loadData()`:**
    *   Determines the path to `metroFinalData.csv` (first checking next to the executable, then a relative path).
    *   Calls `metroSystem_.loadMetroData(filePath, errorMessage);`.
4.  **`MetroSystem::loadMetroData(filename, errorMsg)`:**
    *   Opens `metroFinalData.csv`.
    *   Reads the header line (and prints it if debugging `qDebug` is active).
    *   **Loops through each data line in the CSV:**
        *   Uses `std::stringstream` and `getline(ss, field, ',')` to split the line into 10 expected string fields (FromStation, ToStation, Time, Dist, Cost, SegmentLine, FromLat, FromLon, ToLat, ToLon).
        *   `trim()`s whitespace from each field.
        *   **Validation:** Checks if any of these critical trimmed fields are empty. If so, it prints a warning (if `qDebug` active) and skips the line.
        *   **Conversion & Error Handling (try-catch):**
            *   Tries to convert `timeStr`, `costStr`, `distStr`, and the four coordinate strings into their respective numeric types (`int`, `double`) using `std::stoi` and `std::stod`.
            *   If conversion fails (e.g., non-numeric characters), it catches the exception, prints a warning, and skips the line.
        *   **Populating `stationCoordinates_`:**
            *   For the `fromStation` and `toStation` of the current segment, if their coordinates aren't already in the `stationCoordinates_` map (`std::unordered_map<std::string, QPointF>`), it adds them. `QPointF(longitude, latitude)` is used.
        *   **Populating `graph_`:**
            *   Creates two `Edge` objects (since the graph is undirected): one from `fromStation` to `toStation`, and one from `toStation` to `fromStation`.
            *   The `Edge` stores the `toStation` name, `timeVal`, `distanceVal`, `costVal`, and crucially, the `segmentLine` (from the 6th column of the CSV).
            *   These `Edge` objects are added to the `graph_` (`std::unordered_map<std::string, std::vector<Edge>>`).
        *   **Populating `stationNames_`:** Adds `fromStation` and `toStation` to a `std::set` to keep a unique list of all station names.
    *   After the loop, it checks if `graph_` is empty. If it is (and lines were processed), it sets an error message and returns `false`.
5.  **Back in `MainWindow::loadData()`:**
    *   If `metroSystem_.loadMetroData` was successful, it calls `populateComboBoxes()`.
6.  **`MainWindow::populateComboBoxes()`:**
    *   Gets the sorted list of unique station names from `metroSystem_.getStationNames()`.
    *   Populates the `sourceComboBox_` and `destinationComboBox_`.
    *   Enables UI controls.

**B. User Interaction and Pathfinding (`MainWindow::findPath`)**

1.  **User Selects Source, Destination, Criteria and Clicks "Find Route".**
2.  **`MainWindow::findPath()` is called:**
    *   The `outputOpacityEffect_` for the `QTextEdit` is set to 0 (transparent) in preparation for the fade-in animation.
    *   Basic input validation (are source/destination selected?).
    *   Retrieves selected `sourceStdStr`, `destStdStr`, and `criteriaChoice`.
    *   **Handles Same Source/Destination:** If source and destination are the same, sets a simple HTML message in `outputDisplay_` and returns.
    *   **Calls `MetroSystem` for Pathfinding:**
        *   Based on `criteriaChoice`, it calls one of:
            *   `metroSystem_.findPathLeastStops(sourceStdStr, destStdStr)`
            *   `metroSystem_.findPathByCost(sourceStdStr, destStdStr)`
            *   `metroSystem_.findPathByTime(sourceStdStr, destStdStr)`
        *   These `MetroSystem` methods internally use either BFS or Dijkstra.
3.  **Inside `MetroSystem`'s Pathfinding (e.g., `dijkstra`)**:
    *   The algorithm explores the `graph_`, using `Edge.time` or `Edge.cost` as weights.
    *   It builds up a `parentNode` map to reconstruct the path.
    *   **Path Reconstruction:**
        *   If a path to `end` is found, it traces back from `end` to `start` using `parentNode`.
        *   For each step (e.g., from `prevStation` to `currentStation`):
            *   It calls `findEdge(prevStation, currentStation)` to get the specific `Edge` object that was traversed.
            *   It creates a `PathSegment` object containing `currentStation`'s name, and the `line`, `time`, and `cost` from the found `Edge`.
        *   The start station is added as a `PathSegment` with `isFirstSegment = true`.
        *   The list of `PathSegment` objects is reversed to be in the correct order (start to end) and returned to `MainWindow`.
4.  **Back in `MainWindow::findPath()` - Processing Path Results:**
    *   **If `pathSegments` is empty (no path found):**
        *   Sets an appropriate "No path found" HTML message in `outputDisplay_`.
    *   **If `pathSegments` is NOT empty:**
        *   **HTML Generation for Textual Output:**
            *   Calculates totals (time, cost, stops, line changes) by iterating through `pathSegments`.
            *   Builds a rich HTML string (`htmlOutputContent`) with:
                *   Headers (Route from X to Y, Optimized for Z).
                *   Summary section.
                *   Step-by-step directions, using `PathSegment.stationName`, `PathSegment.lineTakenToReach` (with `getLineColor` for styling), `PathSegment.timeForSegment`, `PathSegment.costForSegment`. It also logic to print "Board Line X" and "Change to Line Y".
        *   **JSON Preparation for Python Script:**
            *   Creates a `QJsonArray` (`pathForPythonJsonArray`).
            *   Iterates through `pathSegments` again.
            *   For each `PathSegment.stationName`, it looks up its coordinates (Longitude, Latitude) from `metroSystem_.getStationCoordinates()`.
            *   Creates a `QJsonObject` for each station: `{"name": "Station Name", "lat": latitude_value, "lng": longitude_value}`.
            *   Adds this `QJsonObject` to the `QJsonArray`.
            *   Converts the `QJsonArray` into a compact JSON string (`jsonDataString`).
        *   **Launch Python Script (`QProcess`):**
            *   Creates a `QStringList pythonArgs`.
            *   First argument: path to `map_generator.py` (derived from `QCoreApplication::applicationDirPath()`).
            *   Second argument: the `jsonDataString`.
            *   Determines the Python executable name (`python3` or `python`).
            *   Calls `pythonMapProcess->startDetached(pythonExecutable, pythonArgs);`. This runs the Python script as a separate, independent process.
5.  **Display Textual Output with Animation (`MainWindow::findPath`)**:
    *   `outputDisplay_->setHtml(htmlOutputContent);` (sets the generated HTML, widget is still transparent).
    *   A `QPropertyAnimation` is created to animate `outputOpacityEffect_`'s `opacity` property from 0.0 to 1.0, making the textual output fade in.

**Part 2: The Python Script (`map_generator.py`)**

1.  **Launched by `QProcess` from C++:**
    *   Receives the JSON string of path data as its first command-line argument (`sys.argv[1]`).
2.  **Argument Parsing:**
    *   `json_arg = sys.argv[1]`
    *   `path_data = json.loads(json_arg)`: Parses the JSON string into a Python list of dictionaries.
3.  **Map Generation (`folium`):**
    *   `metro_map = folium.Map(...)`: Creates a map object, centered (e.g., on the first station or a default), using "OpenStreetMap" tiles by default (no API key needed for this).
    *   **Iterates through `path_data` (list of station dictionaries):**
        *   For each station:
            *   Extracts `lat`, `lng`, and `name`.
            *   `folium.Marker(...).add_to(metro_map)`: Adds a marker for the station on the map with a popup/tooltip.
            *   Collects `(lat, lng)` into a `station_points` list.
    *   **Draws Polyline:**
        *   If `station_points` has at least two points, `folium.PolyLine(locations=station_points, ...).add_to(metro_map)` draws a red line connecting all the station points in sequence.
    *   **Fits Bounds:** `metro_map.fit_bounds(...)` adjusts the map's zoom and center to ensure the entire drawn route is visible.
4.  **Saving and Displaying HTML Map:**
    *   `metro_map.save("generated_route_map.html")`: Saves the Folium map object as an HTML file in the same directory as the Python script (which is the C++ build directory).
    *   `webbrowser.open('file://' + os.path.realpath("generated_route_map.html"))`: Opens this newly created HTML file in the user's default web browser.

**Flow Summary:**

User Input (Qt) -> C++ Pathfinding -> PathSegments (C++) ->
    1.  HTML Generation (C++) -> Display in Qt QTextEdit (with animation)
    2.  JSON Generation (C++) -> Launch Python Script (QProcess) ->
        Python Script Receives JSON -> Folium Generates HTML Map -> Open HTML in Web Browser.

**How to Run**

Most of the GUI part is done with the help of AI, but still knowing the basics of Qt is must.
To run this code you just have to download Qt application, make a new file using Cmake and move all the files where CMakeList exist.
AND MOST IMPORTANTLY - Replace your CMakeList with mine !!!
