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
#include <unordered_map>
#include <chrono>
#include <thread>
#include <sqlite3.h>
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
 * */

// filepath to log file
std::string LOGFILE;
// name of printer
std::string PRINTER;

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

// 'clean' versions of the above labels
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

  "c_ink",
  "m_ink",
  "y_ink",
  "k_ink",
  "lc_ink",
  "lm_ink",
  "ly_ink",
  "lk_ink",
  "w_ink"};


//#####################
// GET LATEST TIME
//#####################

// performs action #1 from above list
// returns latest timestamp in string format 
std::string get_latest_time() {

/* PART 1: DEFINE VARS AND OPEN DB */

  // define database variables
  sqlite3* db;
  char* err_msg = 0;
  int rc = sqlite3_open("sdc_printer.db", &db);
  std::string sql;

  // test if opening the db was successful
  if (rc) {
    std::cerr << "SQLITE3: cannot open database <" << sqlite3_errmsg(db) << '>' << std::endl;
    // cut the program short
    return "EXIT";
  } else {
    std::cout << "SQLITE3: opened database successfully" << std::endl;
  }

  /* PART 2: Create print_jobs table if need be */

  // create print_jobs table if it does not exist
  // here we define the sql statement to do so
  // we will be storing everything as text because
  // it is coming to us as text, and we do not need
  // to do any operations on the values.
  sql = "CREATE TABLE IF NOT EXISTS print_jobs(" \
        "id                      integer NOT NULL PRIMARY KEY AUTOINCREMENT," \
        "printer_name            text    NOT NULL," \
        "job_id                  text    NOT NULL," \
        "job_name                text    NOT NULL," \
        "print_function          text    NOT NULL," \
        "copies_printed          text    NOT NULL," \
        "total_copies            text    NOT NULL," \
        "completed               text    NOT NULL," \
        "canceled                text    NOT NULL," \
        "doublesided             text    NOT NULL," \
        "time_started            text    NOT NULL," \
        "time_duration           text    NOT NULL," \
        "time_units              text    NOT NULL," \
        "image_width             text    NOT NULL," \
        "image_length            text    NOT NULL," \
        "media_length            text    NOT NULL," \
        "prints_per_job          text    NOT NULL," \
        "media_name              text    NOT NULL," \
        "media_integrationid     text    NOT NULL," \
        "type                    text    NOT NULL," \
        "media_width             text    NOT NULL," \
        "media_height            text    NOT NULL," \
        "media_grade             text    NOT NULL," \
        "media_offset            text    NOT NULL," \
        "media_units             text    NOT NULL," \
        "sqft_media_printed      text    NOT NULL," \
        "c_ink                   text    NOT NULL," \
        "m_ink                   text    NOT NULL," \
        "y_ink                   text    NOT NULL," \
        "k_ink                   text    NOT NULL," \
        "lc_ink                  text    NOT NULL," \
        "lm_ink                  text    NOT NULL," \
        "ly_ink                  text    NOT NULL," \
        "lk_ink                  text    NOT NULL," \
        "w_ink                   text    NOT NULL);";

  // execute the statement
  rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &err_msg);
  // test if execution of statement was successful
  if (rc) {
    // either the table could not be created or something else bad happened
    std::cerr << "SQLITE3: cannot create table 'print_jobs' <" << sqlite3_errmsg(db) << '>' << std::endl;
    sqlite3_free(err_msg);
    // cut the program short
    return "EXIT";
  } else {
    // everything is good and we can continue 
    std::cout << "SQLITE3: table 'print_jobs' created successfully (or already exists)" << std::endl; 
  }

  /* PART 3: Check row count int print_jobs table */

  // before we query for the most recent timestamp, we need to
  // make sure that there are at least one rows in the table
  // we will store the row count in this variable
  int row_count = 0;
  // define the row count query
  sql = "SELECT COUNT(*) FROM print_jobs;";
  // define our row count callback as a lambda for easy visibility
  auto rc_callback = [](void* data, int argc, char* argv[], char* col_names[]) -> int { 
    int* row_count = static_cast<int*>(data);
    *row_count = atoi(argv[0]);
    return 0;
  };
  rc = sqlite3_exec(db, sql.c_str(), rc_callback, static_cast<void*>(&row_count), &err_msg);
  if (rc) {
    // something went wrong with our selection
    std::cerr << "SQLITE3: cannot execute row count query <" << sqlite3_errmsg(db) << '>' << std::endl;
    sqlite3_free(err_msg);
    // cut the program short
    return "EXIT";
  } else { 
    std::cout << "SQLITE3: row count query executed successfully, result = <" << row_count << '>' << std::endl; 
    if (row_count == 0) {
      std::cout << "SQLITE3: row count too small to continue - assuming empty table and returning 0" << std::endl;
      return "0";
    } else { 
    
      /* PART 4: Select and return the lastest timestamp */
      
      // we want to query for the highest 'time started' attribute value
      sql = "SELECT MAX(time_started) FROM print_jobs;";
      // we will store the result of the query in this variable
      std::vector<std::string> results; 
      // define our time stamp callback as a lumbda for easy visibility
      auto ts_callback = [](void* data, int argc, char* argv[], char* col_names[]) -> int {
        std::vector<std::string>* results = static_cast<std::vector<std::string>*>(data); 
        if (argc > 0) {
          results->push_back(argv[0]); 
        }
        return 0;
      };
      // execute our query
      rc = sqlite3_exec(db, sql.c_str(), ts_callback, static_cast<void*>(&results), &err_msg);
      // test if execution of statement was successful
      if (rc) {
        // something went wrong with our selection
        std::cerr << "SQLITE3: cannot execute select query <" << sqlite3_errmsg(db) << '>' << std::endl;
        sqlite3_free(err_msg);
        // cut the program short
        return "EXIT";
      } else {
        if (results.size() > 0) {
          std::cout << "SQLITE3: select query executed successfully, result = <" << results[0] << '>' << std::endl;
          // return the latest timestamp 
          return results[0];
        } else {
          // results[0] is NULL, probably because the database has no entries.
          // so we will return 0 to force all block to be captured and inserted
          return "0";
        }
      }
    } 
  } 
}

//#####################
// GET NEW VALUES 
//#####################

// performs actions #2, #3, and #4 from above list
// returns vector<unordered_map> where unordered_map contains new values to insert 
std::vector<std::unordered_map<std::string, std::string>> get_new_values(std::string latest_time) { 
  // this vector will be populated with unordered_maps full of values to insert
  std::vector<std::unordered_map<std::string, std::string>> vals; 
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
            // we will populate this unordered_map with our values and push it into vals
            std::unordered_map<std::string, std::string> block; 
            // iterate through labels array
            for (int i = 0; i < 24; i++) {
              // special case when i == 23
              if (i == 23) {
                // for the last value we simply pull everything past the key to the end of the file
                block.insert({keys[i], line.substr(line.find(labels[i])+labels[i].length())}); 
              } else {
                // find the length of the value using the difference between neighboring labels 
                // -1 because string::find returns the index of the first char in the first match
                len = line.find(labels[i+1]) - line.find(labels[i]) - labels[i].length() - 1;
                block.insert({keys[i], line.substr(line.find(labels[i])+labels[i].length(), len)});
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
              block.insert({keys[24+i], line.substr(line.find(labels[24])+labels[24].length(), len)});      
            } 
            // push our populated map of values from the block into the val vector 
            vals.push_back(block); 
          }
        }
      } 
    } 
  }
  std::cout << "get_new_values(): Found " << vals.size() << " new blocks of data!" << std::endl;
  return vals;
}

//#####################
// INSERT NEW VALUES 
//#####################

// performs action #5 above
void insert_new_values(std::vector<std::unordered_map<std::string, std::string>> vals) {
  // check size of vals vector before we continue
  if (vals.size() == 0) {
    // there are no new values to insert
    std::cout << "insert_new_values: There are no new blocks of data to insert - exiting." << std::endl;
  } else {
    // there are blocks to insert.
    // we begin by opening the database
    // like we did above.  
    sqlite3* db;
    char* err_msg = 0;
    int rc = sqlite3_open("sdc_printer.db", &db);
    std::string sql;

    // test if opening the db was successful
    if (rc) {
      std::cerr << "SQLITE3: cannot open database <" << sqlite3_errmsg(db) << '>' << std::endl; 
    } else {
      std::cout << "SQLITE3: opened database successfully" << std::endl;
      // now we iterate through the vals vector, inserting the values stored in each unordered_map
      for (int i = 0; i < vals.size(); i++) { 
        // first we need to construct the query string
        std::string sql = "INSERT INTO print_jobs (" \
              "printer_name, job_id, job_name, print_function, copies_printed, total_copies, completed," \
              "canceled, doublesided, time_started, time_duration, time_units, image_width," \
              "image_length, media_length, prints_per_job, media_name, media_integrationid, type," \
              "media_width, media_height, media_grade, media_offset, media_units, sqft_media_printed," \
              "c_ink, m_ink, y_ink, k_ink, lc_ink, lm_ink, ly_ink, lk_ink, w_ink)" \
              " VALUES (";
        sql += "'" + PRINTER + "',";                        // printer_name 
        sql += "'" + vals[i]["job_id"] + "',";              // job_id
        sql += "'" + vals[i]["job_name"] + "',";            // job_name
        sql += "'" + vals[i]["print_function"] + "',";      // print_function
        sql += "'" + vals[i]["copies_printed"] + "',";      // copies_printed
        sql += "'" + vals[i]["total_copies"] + "',";        // total_copies
        sql += "'" + vals[i]["completed"] + "',";           // completed
        sql += "'" + vals[i]["canceled"] + "',";            // canceled
        sql += "'" + vals[i]["doublesided"] + "',";         // doublesided
        sql += "'" + vals[i]["time_started"] + "',";        // time_started
        sql += "'" + vals[i]["time_duration"] + "',";       // time_duration
        sql += "'" + vals[i]["time_unites"] + "',";         // time_units
        sql += "'" + vals[i]["image_width"] + "',";         // image_width
        sql += "'" + vals[i]["image_length"] + "',";        // image_length
        sql += "'" + vals[i]["media_length"] + "',";        // media_length
        sql += "'" + vals[i]["prints_per_job"] + "',";      // prints_per_job
        sql += "'" + vals[i]["media_name"] + "',";          // media_name
        sql += "'" + vals[i]["media_integrationid"] + "',"; // media_integrationid
        sql += "'" + vals[i]["type"] + "',";                // type
        sql += "'" + vals[i]["media_width"] + "',";         // media_width
        sql += "'" + vals[i]["media_height"] + "',";        // media_height
        sql += "'" + vals[i]["media_grade"] + "',";         // media_grade
        sql += "'" + vals[i]["media_offset"] + "',";        // media_offset
        sql += "'" + vals[i]["media_units"] + "',";         // media_units
        sql += "'" + vals[i]["sqft_media_printed"] + "',";  // sqft_media_printed
        sql += "'" + vals[i]["c_ink"] + "',";               // c_ink 
        sql += "'" + vals[i]["m_ink"] + "',";               // m_ink  
        sql += "'" + vals[i]["y_ink"] + "',";               // y_ink 
        sql += "'" + vals[i]["k_ink"] + "',";               // k_ink
        sql += "'" + vals[i]["lc_ink"] + "',";              // lc_ink
        sql += "'" + vals[i]["lm_ink"] + "',";              // lm_ink
        sql += "'" + vals[i]["ly_ink"] + "',";              // ly_ink
        sql += "'" + vals[i]["lk_ink"] + "',";              // lk_ink
        sql += "'" + vals[i]["w_ink"] + "'";                // w_ink 
        sql += ");";
        
        // next we execute the statement 
        rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
        if (rc) {
          // something went wrong with our selection
          std::cerr << "SQLITE3: cannot execute insert statement <" << sqlite3_errmsg(db) << '>' << std::endl;
          sqlite3_free(err_msg);
          // cut the program short 
        } else {
          std::cout << "SQLITE3: insert query executed successfully for vals[" << i << ']' << std::endl; 
        }
      }
    }
  }
}


//~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~
//        MAIN 
//~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char* argv[]) { 
  // set global values using cl params
  if (argc < 3) {
    std::cerr << "main(): not enough params, need <filepath> <printer name> - exiting" << std::endl; 
    return 0;
  }
  LOGFILE = std::string(argv[1]);
  PRINTER = std::string(argv[2]);
  std::cout << "main(): LOGFILE = <" << LOGFILE << '>' << std::endl;
  std::cout << "main(): PRINTER = <" << PRINTER << '>' << std::endl;
   
  /*std::string filepath = "print_log.csv";
  char delimiter[] = "|";
  auto foo = parse_csv(filepath, delimiter, 25);
  std::cout << "main(): csv length = " << foo.size() << std::endl;*/

  // run continuously
  while (true) { 
    insert_new_values(get_new_values(get_latest_time()));
    // sleep for 5 minutes
    std::this_thread::sleep_for(std::chrono::seconds(60));
  }
  return 0;
}




