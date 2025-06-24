import sys
import json
import folium
import webbrowser
import os
import tempfile # To create a temporary HTML file

def create_and_display_map(path_data_json_string):
    """
    Generates an HTML map with the given path data using Folium
    and opens it in the default web browser.

    Args:
        path_data_json_string (str): A JSON string representing a list of 
                                     station objects, e.g., 
                                     '[{"name":"Station A","lat":28.6,"lng":77.2}, ...]'
    """
    try:
        path_data = json.loads(path_data_json_string)
        print(f"Python: Successfully parsed JSON data. Number of points: {len(path_data)}")
    except json.JSONDecodeError as e:
        print(f"Python: Error decoding JSON input: {e}", file=sys.stderr)
        print(f"Python: Received string: {path_data_json_string}", file=sys.stderr)
        return

    if not isinstance(path_data, list) or not path_data:
        print("Python: No path data provided or data is not a list. Cannot generate map.", file=sys.stderr)
        return

    # Use the first station as the initial map center, or calculate average
    if path_data:
        initial_lat = path_data[0]['lat']
        initial_lng = path_data[0]['lng']
    else:
        # Default to a generic location if no path data (should not happen if called correctly)
        initial_lat = 28.6139 # Approx center of Delhi
        initial_lng = 77.2090

    # Create a Folium map object.
    # Using OpenStreetMap tiles by default (no API key needed for this)
    # You can explore other tile providers if desired.
    metro_map = folium.Map(location=[initial_lat, initial_lng], zoom_start=12, tiles="OpenStreetMap")

    station_points = []
    for i, station_info in enumerate(path_data):
        try:
            lat = float(station_info['lat'])
            lng = float(station_info['lng'])
            name = station_info.get('name', f"Point {i+1}") # Use 'name' or a default
            
            station_points.append((lat, lng))

            # Add a marker for each station
            folium.Marker(
                location=[lat, lng],
                popup=folium.Popup(f"<b>{name}</b>", max_width=200), # Richer popup
                tooltip=name,
                icon=folium.Icon(color='blue', icon='info-sign' if i == 0 or i == len(path_data) -1 else 'circle', prefix='glyphicon') 
                               # Different icons for start/end
            ).add_to(metro_map)
            print(f"Python: Added marker for {name} at ({lat}, {lng})")

        except KeyError as e:
            print(f"Python: Missing key {e} in station data: {station_info}", file=sys.stderr)
            continue # Skip this station if data is malformed
        except ValueError as e:
            print(f"Python: Could not convert lat/lng to float for station: {station_info}. Error: {e}", file=sys.stderr)
            continue


    # Draw the polyline for the route if there are at least two points
    if len(station_points) >= 2:
        folium.PolyLine(
            locations=station_points,
            color='red',
            weight=5,
            opacity=0.8
        ).add_to(metro_map)
        print(f"Python: Added PolyLine with {len(station_points)} points.")

        # Fit the map to the bounds of the path
        # Create a list of [min_lat, min_lng] and [max_lat, max_lng]
        sw = [min(p[0] for p in station_points), min(p[1] for p in station_points)]
        ne = [max(p[0] for p in station_points), max(p[1] for p in station_points)]
        metro_map.fit_bounds([sw, ne], padding=(0.05, 0.05)) # Add some padding
        print("Python: Map bounds fitted to route.")


    # Save to a temporary HTML file
    try:
        # Create a temporary file that will be deleted when closed (if not using delete=False)
        # Using delete=False so webbrowser can open it before it's gone.
        # We'll have to manage deletion if we want to be very clean, or let OS handle temp files.
        # A simpler approach for now is a fixed name, which gets overwritten.
        
        # temp_html_file = tempfile.NamedTemporaryFile(mode="w", suffix=".html", delete=False, encoding='utf-8')
        # map_file_path = temp_html_file.name
        # temp_html_file.close() # Close it so save can write to it

        # Using a fixed name for simplicity of finding/debugging, will be overwritten
        map_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "generated_route_map.html")
        
        metro_map.save(map_file_path)
        print(f"Python: Map saved to {map_file_path}")

        # Open the HTML file in the default web browser
        webbrowser.open('file://' + os.path.realpath(map_file_path))
        print(f"Python: Attempting to open {map_file_path} in web browser.")

    except Exception as e:
        print(f"Python: Error saving or opening map file: {e}", file=sys.stderr)


if __name__ == "__main__":
    print("Python: map_generator.py script started.")
    if len(sys.argv) > 1:
        json_arg = sys.argv[1]
        print(f"Python: Received argument string of length {len(json_arg)}.")
        # For debugging, print the first and last few chars of the received JSON
        # Be careful if it's very long
        # print(f"Python: Arg snippet: {json_arg[:100]} ... {json_arg[-100:] if len(json_arg) > 200 else ''}")
        create_and_display_map(json_arg)
    else:
        print("Python: No path data argument provided to script.", file=sys.stderr)
        # Example for testing the script directly:
        # test_json_data = '[{"name":"Station Alpha","lat":28.6139,"lng":77.2090},{"name":"Station Beta","lat":28.5240,"lng":77.1855},{"name":"Station Gamma","lat":28.5600,"lng":77.0800}]'
        # print("Python: Running with test data.")
        # create_and_display_map(test_json_data)

    print("Python: map_generator.py script finished.")