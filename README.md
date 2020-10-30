# Concorde_TSP_Utility
Utility to convert Concorde Traveling Salesman Problem (TSP) program files.

### Program CtoK
1. Converts a csv file of decimal degree lat/long coordinates to a tsp file suitable for input to the [Concorde TSP solver](https://www.math.uwaterloo.ca/tsp/concorde.html). Removes any duplicate coordinates found in file. Use the "-o" option for this function.
2. Converts the above csv file and Concorde tour cycle (.cyc) file into a kml file showing the generated tour. Optionally, using the "-n" option, it can produce file with/without waypoint depiction (thumbtacks). 
3. Minimal error checking performed.

Example #1 program invocation:
```text
  C:\> ctok car54r -o
```
Example #2:
```text
  C:\> ctok car54r -n
```
Notes:
* Compiled with MS Visual Studio 2019 Community (v141).
