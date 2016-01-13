// ###################################
// Author: Jacob Paul Adkins
// Contact: jacobpadkins@gmail.com
// Name: sdc_parser
// Desc: Parses log files from printer
// Date: 1/8/2016
// ###################################

/* Inclusions */
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector> 
#include "csv_parser.hpp"

/*
 *
 * The text file we will be parsing contains
 * blocks of text periodically with the format:
 *
 * ###################################
 *
 *  JobID: 244 Job Name: 194_2159_Fish_Fin_156_T1_P1_VuteK_8C.rtl Print Function: 1 Copies Printed: 1 Total Copies: 1 Completed: 1 Canceled: 0 DoubleSided: 0 Time Started: 2016-01-04 16:03:58 Time Duration: 591 Time Units: Seconds Image Width: 115.627 Image Length: 59.5167 Media Length: 59.5167 Prints Per Job: 1 Media Name: 5x10 Media IntegrationId: 0 Media Type: Sheet Media Width: 120 Media Height: 60 Media Grade: 0.01 Media Offset: 0 Media Units: Sqft Media Printed: 47.7897
 * Total Ink Usage:
 * Ink Name: C Ink Consumption: 0.00656397 Ink Units: mL
 * Ink Name: M Ink Consumption: 0.00657145 Ink Units: mL
 * Ink Name: Y Ink Consumption: 0.00657478 Ink Units: mL
 * Ink Name: K Ink Consumption: 0.00657206 Ink Units: mL
 * Ink Name: c Ink Consumption: 0.00657988 Ink Units: mL
 * Ink Name: m Ink Consumption: 0.00655887 Ink Units: mL
 * Ink Name: y Ink Consumption: 0.00655554 Ink Units: mL
 * Ink Name: k Ink Consumption: 0.00656934 Ink Units: mL
 * Ink Name: W Ink Consumption: 0.0131333 Ink Units: mL
 *
 * ###################################
 *
 * The information from these blocks needs to be
 * parsed out and then inserted into a sqlite database.
 * This program will be run periodically on the same 
 * file which is constantly increasing in size so we
 * also need to use the timestamp from the field
 * "Time Started" to determine which blocks we need to
 * parse and insert and which we should ignore because
 * they have already been processed.
 *
 * This program is structured like so:
 * 1. aquire timestamp of most recent insertion from db
 * 2. load file and parse until we reach a block of interest
 * 3. check timestamp of block and ignore if need be
 * 4. continue to the end of the file, gathering any new data
 * 5. finally, insert new data into the db
 *
 *
 *
 * !!!
 * THIS VERSION OF MAIN HAS THE DATABASE FEATURES DISABLED
 * instead you pass a single cl param for the starting 
 * cutoff date, and all jobs with timestamps after that date are
 * parsed and output to a local .csv file
 * */

// filepath to log file
std::string LOGFILE;
// name of printer
std::string PRINTER;
// timestamp min cutoff
std::string TIMECUT;
// output csv file
std::string OUTFILE;

// define value field labels
const std::string labels[] = {
  "JobID: ",                    // 0
  "Job Name: ",                 // 1
  "Print Function: ",           // 2
  "Copies Printed: ",           // 3
  "Total Copies: ",             // 4
  "Completed: ",                // 5
  "Canceled: ",                 // 6
  "DoubleSided: ",              // 7
  "Time Started: ",             // 8
  "Time Duration: ",            // 9
  "Time Units: ",               // 10
  "Image Width: ",              // 11
  "Image Length: ",             // 12
  "Media Length: ",             // 13
  "Prints Per Job: ",           // 14
  "Media Name: ",               // 15
  "Media IntegrationId: ",      // 16
  "Type: ",                     // 17
  "Media Width: ",              // 18
  "Media Height: ",             // 19
  "Media Grade: ",              // 20
  "Media Offset: ",             // 21
  "Media Units: ",              // 22
  "Sqft Media Printed: ",       // 23
  "Ink Consumption: ",          // 24
  "Ink Units: "};               // 25

//#####################
// GET NEW VALUES 
//#####################

//* NOTE: some of the comments here might
// not be correct. This was adapted from Main()
// to work with gen_csv()  

// performs actions #2, #3, and #4 from above list
// returns map of new values to insert 
std::map<std::string, std::vector<std::string>> get_new_values(std::string latest_time) { 
  // this vector will be populated with unordered_maps full of values to insert
  std::map<std::string, std::vector<std::string>> vals; 
  // check for error cade in latest_time
  if (latest_time != "EXIT") {
    // create input stream from log file
    std::ifstream ifs(LOGFILE);
    // make sure the file could be opened correctly
    if (!ifs.is_open()) {
      std::cerr << "get_new_values(): Cannot open log file <" << LOGFILE << "> - exiting";
      return vals;
    }
    // define key phrase that tells us we have reached a block to parse
    std::string key_phrase = "Job Complete Data:"; 
    
    // loop until we reach the end of ifs
    while(!ifs.eof()) {
      // line will hold the current line
      std::string line;
      // grab lines delimited by \n
      std::getline(ifs, line);
      // check if key_phrase exists in the current line
      if (line.find(key_phrase) != std::string::npos) {
        // line now contains a string containing the timestamp
        std::getline(ifs, line);
        // first we want to determine if this is just a test print
        // by checking the "Print Name" value which is labels[1]
        int pos = line.find(labels[1])+labels[1].length();
        if (line.substr(pos, 15) != "Test Check Jets") {
          // next, grab the timestamp and check it against the previous lastest time
          int len = line.find(labels[9]) - labels[9].length() - line.find(labels[8]);
          std::string t = line.substr(line.find(labels[8])+labels[8].length(), len);
          // check if this block is more recent than the last one inserted into the db
          if (t > latest_time) { 
            // iterate through labels array
            for (int i = 0; i < 24; i++) {
              // special case when i == 23
              if (i == 23) {
                // for the last value we simply pull everything past the key to the end of the file
                //block.insert({keys[i], line.substr(line.find(labels[i])+labels[i].length())}); 
                vals[keys[i]].push_back(line.substr(line.find(labels[i])+labels[i].length()));
              } else {
                // find the length of the value using the difference between neighboring labels 
                // -1 because string::find returns the index of the first char in the first match
                len = line.find(labels[i+1]) - line.find(labels[i]) - labels[i].length() - 1;
                //block.insert({keys[i], line.substr(line.find(labels[i])+labels[i].length(), len)});
                vals[keys[i]].push_back(line.substr(line.find(labels[i])+labels[i].length(), len));
              }
            } 
            // get rid of the 'Total Ink Usage:' line
            std::getline(ifs, line); 
            // now grab ink using values from next 9 lines
            for (int i = 0; i < 9; i++) {
              // first get the next line
              std::getline(ifs, line);
              // define len
              len = line.find(labels[25]) - line.find(labels[24]) - labels[24].length() -1;
              // then grab and insert the value 
              //block.insert({keys[24+i], line.substr(line.find(labels[24])+labels[24].length(), len)});      
              vals[keys[24+i]].push_back(line.substr(line.find(labels[24])+labels[24].length(), len));
            } 
            // push our populated map of values from the block into the val vector 
            //vals.push_back(block); 
          }
        }
      } 
    } 
  }
  std::cout << "get_new_values(): Found " << vals[keys[0]].size() << " new blocks of data!" << std::endl;
  return vals;
}

//~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~
//        MAIN 
//~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char* argv[]) { 
  // set global values using cl params
  if (argc < 5) {
    std::cerr << "main(): not enough params, need <filepath> <printer name> " \
        "<YYYY-MM-DD HH:MM:SS (army time)> <<output file> - exiting" << std::endl; 
    return 0;
  }

  LOGFILE = std::string(argv[1]);
  PRINTER = std::string(argv[2]);
  TIMECUT = std::string(argv[3]);
  OUTFILE = std::string(argv[4]);
  std::cout << "main(): LOGFILE = <" << LOGFILE << '>' << std::endl;
  std::cout << "main(): PRINTER = <" << PRINTER << '>' << std::endl;
  std::cout << "main(): TIMECUT = <" << TIMECUT << '>' << std::endl;
  std::cout << "main(): OUTFILE = <" << OUTFILE << '>' << std::endl; 

  gen_csv(get_new_values(TIMECUT), OUTFILE); 
  return 0;
}

