#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdlib>

using namespace std;

#include "tube.h"

/* You are pre-supplied with the functions below. Add your own 
   function definitions to the end of this file. */

/* internal helper function which allocates a dynamic 2D array */
char **allocate_2D_array(int rows, int columns) {
  char **m = new char *[rows];
  assert(m);
  for (int r=0; r<rows; r++) {
    m[r] = new char[columns];
    assert(m[r]);
  }
  return m;
}

/* internal helper function which deallocates a dynamic 2D array */
void deallocate_2D_array(char **m, int rows) {
  for (int r=0; r<rows; r++)
    delete [] m[r];
  delete [] m;
}

/* internal helper function which gets the dimensions of a map */
bool get_map_dimensions(const char *filename, int &height, int &width) {
  char line[512];
  
  ifstream input(filename);

  height = width = 0;

  input.getline(line,512);  
  while (input) {
    if ( (int) strlen(line) > width)
      width = strlen(line);
    height++;
    input.getline(line,512);  
  }

  if (height > 0)
    return true;
  return false;
}

/* pre-supplied function to load a tube map from a file*/
char **load_map(const char *filename, int &height, int &width) {

  bool success = get_map_dimensions(filename, height, width);
  
  if (!success)
    return NULL;

  char **m = allocate_2D_array(height, width);
  
  ifstream input(filename);

  char line[512];
  char space[] = " ";

  for (int r = 0; r<height; r++) {
    input.getline(line, 512);
    strcpy(m[r], line);
    while ( (int) strlen(m[r]) < width )
      strcat(m[r], space);
  }
  
  return m;
}

/* pre-supplied function to print the tube map */
void print_map(char **m, int height, int width) {
  cout << setw(2) << " " << " ";
  for (int c=0; c<width; c++)
    if (c && (c % 10) == 0) 
      cout << c/10;
    else
      cout << " ";
  cout << endl;

  cout << setw(2) << " " << " ";
  for (int c=0; c<width; c++)
    cout << (c % 10);
  cout << endl;

  for (int r=0; r<height; r++) {
    cout << setw(2) << r << " ";    
    for (int c=0; c<width; c++) 
      cout << m[r][c];
    cout << endl;
  }
}

/* pre-supplied helper function to report the errors encountered in Question 3 */
const char *error_description(int code) {
  switch(code) {
  case ERROR_START_STATION_INVALID: 
    return "Start station invalid";
  case ERROR_ROUTE_ENDPOINT_IS_NOT_STATION:
    return "Route endpoint is not a station";
  case ERROR_LINE_HOPPING_BETWEEN_STATIONS:
    return "Line hopping between stations not possible";
  case ERROR_BACKTRACKING_BETWEEN_STATIONS:
    return "Backtracking along line between stations not possible";
  case ERROR_INVALID_DIRECTION:
    return "Invalid direction";
  case ERROR_OFF_TRACK:
    return "Route goes off track";
  case ERROR_OUT_OF_BOUNDS:
    return "Route goes off map";
  }
  return "Unknown error";
}

/* presupplied helper function for converting string to direction enum */
Direction string_to_direction(const char *token) {
  const char *strings[] = {"N", "S", "W", "E", "NE", "NW", "SE", "SW"};
  for (int n=0; n<8; n++) {
    if (!strcmp(token, strings[n])) 
      return (Direction) n;
  }
  return INVALID_DIRECTION;
}


bool get_symbol_position( char **map, const int height, const int width,
			  const char target, int &r, int &c ) {
  for ( r = 0; r < height; ++r ) 
    for ( c = 0; c < width; ++c ) 
      if (map[r][c] == target)
	return true;

  r = -1;
  c = -1;
  return false;
}

bool get_symbol_from( const char* filename, const char *name, char &symbol ) {
  const int MAX_LINE_SIZE = 512;
  const int NAME_INDEX_ON_LINE = 2;

  ifstream in;
  in.open(filename);
  if (in.fail()) {
    cout << "Sorry, the file couldn't be opened!\n";
    exit(1); 
  }
 
  char line_buffer[MAX_LINE_SIZE];
  in.getline(line_buffer, MAX_LINE_SIZE - 1);
  while (!in.eof()) {
    if (strcmp(line_buffer + NAME_INDEX_ON_LINE, name) == 0) {
      symbol = *line_buffer;
      in.close();
      return true;
    }
    in.getline(line_buffer, MAX_LINE_SIZE - 1);
  }
   
  symbol = ' ';
  return false;
}
  
char get_symbol_for_station_or_line( const char *name ) {
  char symbol;
  if ( !get_symbol_from("stations.txt",name,symbol) )
    get_symbol_from("lines.txt",name,symbol);

  return symbol;
}

void get_direction( const char *&route, char *direction_string ) {
  int n = 0;
  while ( *route != ',' && *route != '\0' ) {
    direction_string[n] = *route;
    ++route;
    ++n; 
  }
  direction_string[n] = '\0';
  if ( *route == ',' )
    ++route;
}

void get_station_name( const char target, char *destination ) {
  const int MAX_LINE_SIZE = 512;
  const int NAME_INDEX_ON_LINE = 2;

  ifstream in;
  in.open("stations.txt");
  if (in.fail()) {
    cout << "Sorry, the file couldn't be opened!\n";
    exit(1); 
  }
 
  char line_buffer[MAX_LINE_SIZE];
  in.getline(line_buffer, MAX_LINE_SIZE - 1);
  while (!in.eof()) {
    if (target == *line_buffer) {
      strcpy(destination, line_buffer + NAME_INDEX_ON_LINE);
      in.close();
      return;
    }
    in.getline(line_buffer, MAX_LINE_SIZE - 1);
  }
  in.close();
}
  
int recurse_through_route( char **map, const int height, const int width,
			   int prev2_row, int prev2_col, int prev2_symbol,
			   int prev_row, int prev_col, char prev_symbol,
			   int curr_row, int curr_col, char curr_symbol, 
			   const char *route, char *destination ) {
 
  char direction_string[3];
  get_direction(route, direction_string);
  
  Direction direction = string_to_direction(direction_string);
  if ( direction == INVALID_DIRECTION )
    return ERROR_INVALID_DIRECTION;

  prev2_row = prev_row;
  prev2_col = prev_col;
  prev2_symbol = prev_symbol;
  
  prev_row = curr_row;
  prev_col = curr_col;
  prev_symbol = curr_symbol;

  switch(direction) {
  case N: curr_row--; break;
  case S: curr_row++; break;
  case W: curr_col--; break;
  case E: curr_col++; break;
  case NE: curr_row--; curr_col++; break;
  case NW: curr_row--; curr_col--; break;
  case SE: curr_row++; curr_col++; break;
  case SW: curr_row++; curr_col--; break;
  default: cout << "Error!\n"; exit(1);
  }

  if (curr_row < 0 || curr_row >= height || 
      curr_col < 0 || curr_col >= width )
    return ERROR_OUT_OF_BOUNDS;
  
  curr_symbol = map[curr_row][curr_col];
  if (curr_symbol == ' ')
    return ERROR_OFF_TRACK;

  if ( curr_symbol != prev_symbol &&
       !isalnum(curr_symbol) && 
       !isalnum(prev_symbol) )
    return ERROR_LINE_HOPPING_BETWEEN_STATIONS;

  if ( curr_row == prev2_row && curr_col == prev2_col && !isalnum(prev_symbol) )
    return ERROR_BACKTRACKING_BETWEEN_STATIONS;

  if (*route == '\0') {
    if ( !isalnum(curr_symbol) )
      return ERROR_ROUTE_ENDPOINT_IS_NOT_STATION;
    else
      get_station_name(curr_symbol,destination);
  }
  
  bool moved_to_new_line =
    curr_symbol != prev2_symbol &&
    isalnum(prev_symbol) &&
    !isalnum(curr_symbol) &&
    !isalnum(prev2_symbol) ;
  
  int recursive_call;
  if (*route == '\0')
    recursive_call = 0;
  else
    recursive_call = recurse_through_route( map, height, width, 
					    prev2_row, prev2_col, prev2_symbol,
					    prev_row,  prev_col,  prev_symbol,
					    curr_row,  curr_col,  curr_symbol, 					      route, destination );
  
  if ( !moved_to_new_line || recursive_call < 0 )
    return recursive_call;
  else
    return ++recursive_call;
}
   
int validate_route( char **map, const int height, const int width, 
		    const char *start_station, const char *route, 
		    char *destination ) {
  
  char start_station_symbol;
  if ( !get_symbol_from("stations.txt",start_station,start_station_symbol) )
    return ERROR_START_STATION_INVALID;

  int row, col;
  get_symbol_position( map, height, width, start_station_symbol, row, col );
  
  return recurse_through_route( map, height, width, 
				row, col, start_station_symbol, 
				row, col, start_station_symbol, 
				row, col, start_station_symbol, 
				route, destination );
}
  

      
