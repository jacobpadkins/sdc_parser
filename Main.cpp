// ###################################
// Author: Jacob Adkins
// Name: sdc_parser
// Desc: Parses log files from printer
// Date: 1/8/2016
// ###################################

/* Inclusions */
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

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
 */

// define value field delimeters
std::vector<std::string> delimeters = {" JobID: ", " Job Name: ", " Print Function: ", " Copies Printed: ",
 " Total Copies: ", " Completed: ", " Cancelled: ", " DoubleSided: ", " Time Started: ", " Time Duration: ",
 " Time Units: ", " Image Width: ", " Image Length: ", " Media Length: ", " Prints Per Job: ", " Media Name: ",
 " Media IntegrationId: ", " Type: ", " Media Width: ", " Media Height: ", " Media Grade: ", " Media Offset: ",
 " Media Units: ", " Sqft Media Printed: "};

// performs action #1 from above list
// returns latest timestamp in string format
std::string get_latest_date();

// performs actions #2, #3, and #4 from above list
// returns vector<unordered_map> where unordered_map contains new values to insert
std::vector<std::unordered_map<std::string, std::string>> get_new_values(std::string latest_time);

// performs action #5 above
void insert_new_values(std::vector<std::unordered_map<std::string, std::string>>);

int main() {
 // create input stream from log file
 std::ifstream ifs("jdfserverd.log");
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
   // by checking the "Print Name" value
  }
 }
}
