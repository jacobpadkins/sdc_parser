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
#include <sqlite3.h>
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
const std::string delims[] = {
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
 "Sqft Media Printed: "};      // 23

// 'clean' versions of the above delims
// for use as keys and attribute names
const std::string keys[] = {
 "job_id",                      // 0
 "job_name",                    // 1
 "print_function",              // 2
 "copies_printed",              // 3
 "total_copies",                // 4
 "completed",                   // 5
 "cancelled",                   // 6
 "doublesided",                 // 7
 "time_started",                // 8
 "time_duration",               // 9
 "time_units",                  // 10
 "image_width",                 // 11
 "image_length",                // 12
 "media_length",                // 13
 "prints_per_job",              // 14
 "media_name",                  // 15
 "media_integrationid",         // 16
 "type",                        // 17
 "media_width",                 // 18
 "media_height",                // 19
 "media_grade",                 // 20
 "media_offset",                // 21
 "media_units",                 // 22
 "sqft_media_printed",          // 23

 // ink keys

 "C_ink",
 "M_ink",
 "Y_ink",
 "K_ink",
 "c_ink",
 "m_ink",
 "y_ink",
 "k_ink",
 "W_ink"};
 
// performs action #1 from above list
// returns latest timestamp in string format
std::string get_latest_time();

// performs actions #2, #3, and #4 from above list
// returns vector<unordered_map> where unordered_map contains new values to insert
std::vector<std::unordered_map<std::string, std::string>> get_new_values(std::string latest_time);

// performs action #5 above
void insert_new_values(std::vector<std::unordered_map<std::string, std::string>> vals);
 
int main(int argc, char* argv[]) {
 std::vector<std::unordered_map<std::string, std::string>> new_values = get_new_values("0"); 
 return 0;
}

/*

// define get_latest_date()
std::string get_latest_time() { 
 // define database variables
 sqlite3* db;
 char* zErrMsg = 0;
 int rc = sqlite3_open("test.db", &db);
 char* sql;

 // test if opening the db was successful
 if (rc) {
  std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
  // cuts the program short
  return "EXIT";
 } else {
  std::cout << "Opened database successfully" << std::endl;
 }
 
 // create print_jobs table if it does not exist
 // here we define the sql statement to do so
 sql = "CREATE TABLE IF NOT EXISTS print_jobs(" \
       "id      int     NOT NULL        PRIMARY KEY," \
       "

 sqlite3_close(db);
 return "EXIT";
}

*/

// define get_new_values()
std::vector<std::unordered_map<std::string, std::string>> get_new_values(std::string latest_time) { 
 // this vector will be populated with unordered_maps full of values to insert
 std::vector<std::unordered_map<std::string, std::string>> vals; 
 // check for error cade in latest_time
 if (latest_time != "EXIT") {
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
    // by checking the "Print Name" value which is delims[1]
    int pos = line.find(delims[1])+delims[1].length();
    if (line.substr(pos, 15) != "Test Check Jets") {
     // next, grab the timestamp and check it against the previous lastest time
     int len = line.find(delims[9]) - delims[9].length() - line.find(delims[8]);
     std::string t = line.substr(line.find(delims[8])+delims[8].length(), len);
     // check if this block is more recent than the last one inserted into the db
     if (t > latest_time) { 
      // we will populate this unordered_map with our values and push it into vals
      std::unordered_map<std::string, std::string> block;
      // iterate through delimeters array
      for (int i = 0; i < 24; i++) {
       // special case when i == 23
       if (i == 23) {
        // for the last value we simply pull everything past the key to the end of the file
        block.insert({keys[i], line.substr(line.find(delims[i])+delims[i].length())}); 
       } else {
        // find the length of the value using the difference between neighboring delimeters
        // -1 because string::find returns the index of the first char in the first match
        len = line.find(delims[i+1]) - line.find(delims[i]) - delims[i].length() - 1;
        block.insert({keys[i], line.substr(line.find(delims[i])+delims[i].length(), len)});
       }
      } 
      // push our populated map of values from the block into the val vector 
      vals.push_back(block); 
     }
    }
   } 
  } 
 }
 return vals;
}
