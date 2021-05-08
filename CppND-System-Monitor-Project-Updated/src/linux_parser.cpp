#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;


template <typename RetType> RetType searchByKey(string const& check, string const& path) {
  string line;
  string key;
  RetType value;
  std::ifstream filestream(LinuxParser::kProcDirectory + path);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == check) {
          filestream.close();
          return value;
        }
      }
    }
  filestream.close();
  } 
  return value;
};


template <typename RetType> RetType searchFromFile(string const& path) {
  string line;
  RetType value;
  std::ifstream filestream(LinuxParser::kProcDirectory + path);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> value;
    filestream.close();
  }
  return value;
};


// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}


// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string memTotal_check = "MemTotal:";
  string memFree_check = "MemFree:";
  float total = searchByKey<float> (memTotal_check, kMeminfoFilename);
  float free = searchByKey<float> (memFree_check, kMeminfoFilename);
  return (total - free) / total;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  return searchFromFile<long> (kUptimeFilename);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line;
    std::getline(filestream, line);
    string cpu;
    long user;
    long nice;
    long system;
    long idle;
    long iowait;
    long irq;
    long softirq;
    long steal;
    long guest;
    long gustnice;
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> gustnice;
    return user + nice + system + irq + softirq + steal + idle + iowait;
  }
  return 0;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies() {
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line;
    std::getline(filestream, line);
    string cpu;
    long user;
    long nice;
    long system;
    long idle;
    long iowait;
    long irq;
    long softirq;
    long steal;
    long guest;
    long gustnice;
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> gustnice;
    return user + nice + system + irq + softirq + steal;
  }
  return 0;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string ignore;
  string utime, stime, cutime, cstime;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      for (int i = 0; i < 13; i++)
        linestream >> ignore;
      linestream >> utime >> stime >> cutime >> cstime;
      long totalTime = std::stol(utime) + std::stol(stime) + std::stol(cutime) + std::stol(cstime);
      return totalTime;
    }
  }
  return 0;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return Jiffies() - ActiveJiffies(); }

// TODO: Read and return CPU utilization
float LinuxParser::CpuUtilization() { return (float)ActiveJiffies() / (float)Jiffies(); }

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string key = "processes";
  int processes = searchByKey<int> (key, kStatFilename);
  return processes; 
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string key = "procs_running";
  int processes = searchByKey<int> (key, kStatFilename);
  return processes;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string command = searchFromFile<string> (std::to_string(pid) + kCmdlineFilename);
  return command;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string key = "VmSize:";
  string ram = searchByKey<string> (key, std::to_string(pid) + kStatusFilename);
  return std::to_string(std::stol(ram)/ 1024);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string key = "Uid:";
  string uid = searchByKey<string> (key, std::to_string(pid) + kStatusFilename);
  return uid;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  auto uid = Uid(pid);
  string line;
  string user;
  const string delimiter = ":";
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      size_t start = 0;
      vector<string> tokens;
      tokens.reserve(3);
      for (int i = 0; i < 3; ++i) {
        auto end = line.find(delimiter, start);
        tokens.push_back(line.substr(start, end - start));
        start = end + delimiter.size();
      }
      if (tokens[2] == uid) {
        user = tokens[0];
        return user;
      }
    }
  }
  return user;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  string value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    for (int i = 0; i < 22; ++i) {
      filestream >> value;
    }
    return LinuxParser::UpTime() - std::stol(value) / sysconf(_SC_CLK_TCK);
  }
  return 0;
}