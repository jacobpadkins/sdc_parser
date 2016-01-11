/*
 * This is a simple csv parser/writer and will be incorperated 
 * into features of the main program later on.
 *
 */

#pragma once

/* Inclusions */
#include <vector>
#include <map>
#include <iostream>
#include <fstream> 
#include <string.h>

// this function parses a csv file and returns the rows in a map
std::map<std::string, std::vector<std::string>>
parse_csv(std::string filename, char* delimiter, int column_count) { 
  std::map<std::string, std::vector<std::string>> csv;

  // create ifstream file filename and check if open
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    std::cerr << "parse_csv(): unable to open file <" << filename << '>' << std::endl;
    return csv;
  }

  // first we populate csv with column names and empty vectors 
  std::string line;
  std::getline(ifs, line);
  char* c_line = new char[line.length() + 1];
  char* token;
  
  strcpy(c_line, line.c_str());
  token = std::strtok(c_line, delimiter); 
  while (token != NULL) {
    std::vector<std::string> v;
    csv.insert({std::string(token), v});
    token = std::strtok(NULL, delimiter);
  }
  delete[] c_line; 
  
  // now we iterate through the rest of the csv, pushing the values into
  // their respective column value vectors
  while (!ifs.eof()) {
    std::getline(ifs, line);
    c_line = new char[line.length() + 1];
    token = std::strtok(c_line, delimiter);
    while (token != NULL) {
      token = std::strtok(NULL, delimiter);
    }
    delete[] c_line;
  }

  return csv;
}
