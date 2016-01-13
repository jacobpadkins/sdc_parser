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

// 'clean' versions of the above labels
// for use as keys and attribute names
const std::string keys[] = {
  "job_id",                     // 0
  "job_name",                   // 1
  "print_function",             // 2
  "copies_printed",             // 3
  "total_copies",               // 4
  "completed",                  // 5
  "cancelled",                  // 6
  "doublesided",                // 7
  "time_started",               // 8
  "time_duration",              // 9
  "time_units",                 // 10
  "image_width",                // 11
  "image_length",               // 12
  "media_length",               // 13
  "prints_per_job",             // 14
  "media_name",                 // 15
  "media_integrationid",        // 16
  "type",                       // 17
  "media_width",                // 18
  "media_height",               // 19
  "media_grade",                // 20
  "media_offset",               // 21
  "media_units",                // 22
  "sqft_media_printed",         // 23

 // ink keys

  "c_ink",                      // 24
  "m_ink",                      // 25
  "y_ink",                      // 26
  "k_ink",                      // 27
  "lc_ink",                     // 28
  "lm_ink",                     // 29
  "ly_ink",                     // 30
  "lk_ink",                     // 31
  "w_ink"};                     // 32



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

// this function takes a map of vectors and generates a csv
void gen_csv(std::map<std::string, std::vector<std::string>> data, std::string out) { 
  // open ofstream for output
  std::ofstream ofs(out);
  if (!ofs.is_open()) {
    std::cerr << "gen_csv: Cannot create/open output file <" << out << "> - exiting";
    return; 
  }

  // iterate through keys, creating column value line
  std::string column_row = "";
  char delim = '|';

  for (int i = 0; i < 33; i++) {
    column_row += keys[i] + delim;
  }
  
  column_row = column_row.substr(0, column_row.size()-1);
  column_row += '\n';
  ofs << column_row;
 
  // iterate through vectors, creating each subsequent line 
  std::string data_row = "";
  for (int i = 0; i < data[keys[i]].size(); i++) {
    for (int j = 0; j < data.size(); j++) {
      data_row += data[keys[j]][i] + delim;
    }
    data_row = data_row.substr(0, column_row.size()-1);
    data_row += '\n';
    ofs << data_row;
    data_row = "";
  } 
}
