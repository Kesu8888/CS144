//
// Created by peterill on 8/4/24.
//
#include "set"
#include "iostream"
#include "fstream"
#include "string"
using namespace std;

int main(int argc, char* argv[]) {
  ifstream data("data.txt");
  string line;
  set<int> list;
  getline(data, line);
  int i = 0;
  while (true) {
    if (line.at(i) == 't') {
      if (line.at(i+1) == 'i') {
        break;
      }
    }
    i++;
  }
  ofstream output("output.txt");
  do {
    string num;
    while (line.at(i) != 't')
      i++;
    int j = i+5;
    while (line.at(j) != ' ') {
      num.push_back(line.at(j));
      j++;
    }
    output << num << "\n";
  } while ( getline(data, line));


  //The below code is to get total unique sequence numbers received
  /*while ( getline(data, line)) {
    string num;
    int j = i;
    while (line.at(j) != ' ') {
      num.push_back(line.at(j));
      j++;
    }
    list.insert( stoi(num));
  }
  data.close();
  cout << "Total unique number" << list.size() << "\n";

  size_t cur = 0;
  int max = 0;
  for (int l = 1; l <= 38749; l++) {
    if (!list.count(l)) {
      int missmax = 1;
      while (!list.count(l+1)) {
        l++;
        missmax++;
      }
      max = std::max(max, missmax);
      cur += 1;
    }
  }
  //cur = list.size()/cur;
  cout << "max consecutive sequence is " << cur << "\n" << "miss max = " << max << "\n";*/
}