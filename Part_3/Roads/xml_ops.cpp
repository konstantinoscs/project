#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "../lib/curve.h"
//#include "../lib/distance_ops.h"
#include "road.h"

using namespace std;

void check_highway(string & type){
  if(!type.compare("motorway") || !type.compare("primary")){
    return ;
  }
  else if(!type.compare("residential") || !type.compare("secondary")){
    return ;
  }
  else if(!type.compare("service") || !type.compare("tertiary")){
    return ;
  }
  else if(!type.compare("trunk") || !type.compare("unclassified")){
    return ;
  }
  type.clear();
}

int compare(const string & s1, const string & s2){
  size_t l1{s1.length()}, l2{s2.length()};
  if(l1 > l2)
    return 1;
  else if (l2 > l1)
    return -1;
  else
    return s1.compare(s2);
}

size_t binary_search(vector<node> & nodes, string id){
  size_t l{0}, r{nodes.size()-1}, i{(l+r)/2};
  node r_node;
  int comp = compare(nodes[i].id, id);
  while(comp){
    //cout << "Searching " << id << " comparing with " << nodes[i].id << " i " << i;
    //cout << " l,r " << l << " " << r << ' ' << comp << endl;
    if(comp > 0)
      r = --i;
    else
      l = ++i;
    i = (l+r)/2;
    comp = compare(nodes[i].id, id);
  }
  //cout << "size " << nodes.size() << " i " << i  << " id " << id << '\n';
  //nodes[i].print();
  return i;
}

//write nodes writes the athens.csv file with the roads info
//and returns a vector of the indexes of a way's nodes in the
//general nodes vector
inline vector<int> write_nodes(ofstream & out, vector<node> & nodes, vector<string> & nds){
  size_t target{0};
  vector<int> node_i;
  for(size_t i=0; i<nds.size(); i++){
    target = binary_search(nodes, nds[i]);
    out << ", " << nodes[target].lat << ", " << nodes[target].lon;
    //mark that we've encountered this node before in a road
    nodes[target].refs++;
    node_i.push_back(target);
  }
  return node_i;
}

bool parse_xml(vector<road> & roads, vector<node> & nodes, const string &data_s,
  const string &out_s){
  ifstream data(data_s);
  ofstream out(out_s);
  vector<string> nds;
  node t_node;
  road t_road;
  string temp{}, type{}, id{};
  int counter{0};
  size_t max = std::numeric_limits<streamsize>::max();
  nodes.reserve(3004103);
  roads.reserve(89701);
  //ignore the first 3 lines of the xml file
  data.ignore(max, '\n');
  data.ignore(max, '\n');
  data.ignore(max, '\n');
  //start reading nodes;
  while(true){
    data >> temp;
    //if it's not a node tag break
    if(!temp.compare("<node")){
      data >> temp;
      //copy the id
      t_node.id.assign(temp, 4, temp.length()-5);
      //parse latitude
      data >> temp;
      t_node.lat = stod(temp.substr(5));
      //parse longitute
      data >> temp;
      t_node.lon = stod(temp.substr(5));
      //move node to the vector
      nodes.push_back(move(t_node));
      //cout << "Node read" << endl;
      t_node.id.clear();
      counter++;
    }
    else if(!temp.compare("<way")){
      break;
    }
    //ignore the rest of the line
    data.ignore(max, '\n');
  }

  //start reading roads
  while(true){
    if(!temp.compare("<way")){
      data >> temp;
      id.assign(temp, 4, temp.length()-5);
      //id = temp.substr(4, temp.length()-5);
    }
    else if(!temp.compare("<nd")){
      data >> temp;
      nds.push_back(move(temp.substr(5, temp.length()-8)));
    }
    else if(!temp.compare("<tag")){
      data >> temp;
      if(!temp.compare("k=\"highway\"")){
        data >> temp;
        type = temp.substr(3, temp.length()-6);
        //check if it's a highway we're interested in
        check_highway(type);
      }
    }
    else if(!temp.compare("</way>")){
      //write road to file if valid
      if(!type.empty()){
        out << id << ", " << type;
        t_road.nodes = write_nodes(out, nodes, nds);
        out << endl;
        t_road.id = move(id);
        t_road.type = move(type);
        roads.push_back(move(t_road));
      }
      id.clear();
      type.clear();
      nds.clear();
    }
    else if(!temp.compare("<relation")){
      //relation tags are in the end of the file so we're done
      break;
    }
    data.ignore(max, '\n');
    data >> temp;
  }
  return true;
}

//index takes a type of way and returns its index in the vector
//where segments of this type of way are stored
inline size_t index(string type){
  static string types[] {
    "motorway", "primary", "residential",
    "secondary", "service", "tertiary",
    "trunk", "unclassified"};
    for(size_t i=0; i<7; i++)
      if(!types[i].compare(type))
        return i;
}

double euclid_dist(double x1, double y1, double x2, double y2){
	double temp{}, ed{};
	temp = x1 - x2;
	ed += temp * temp;
	temp = y1 - y2;
	ed += temp * temp;
	return sqrt(ed);
}

inline double curvature(double l1, double l2, double l3){
  return l1*l2*l3/sqrt((l1+l2+l3)*(l2+l3-l1)*(l3+l1-l2)*(l1+l2-l3));
}

void make_segments(vector<road> &roads, vector<node> &nodes, const string &out_s){
  ofstream out(out_s);
  double l1{}, l2{}, l3{}, curb{}, thrs{0.1};
  int segid{}, nsize{}, nthrs{200};
  int maxsize{}, minsize{std::numeric_limits<int>::max()};
  vector<double> coords;
  cout << "Curvatures:\n";
  for(size_t i=0; i<roads.size(); i++){
    nsize = 2;
    cout << "new road" << endl;
    coords.push_back(nodes[roads[i].nodes[0]].lat);
    coords.push_back(nodes[roads[i].nodes[0]].lon);
    if(roads[i].nodes.size() ==1){
      continue;
    }
    for(size_t j=0; j<roads[i].nodes.size()-2; j++){
      l1 = euclid_dist(nodes[roads[i].nodes[j]].lat, nodes[roads[i].nodes[j]].lon,
        nodes[roads[i].nodes[j+1]].lat, nodes[roads[i].nodes[j+1]].lon);
      l2 = euclid_dist(nodes[roads[i].nodes[j+1]].lat, nodes[roads[i].nodes[j+1]].lon,
        nodes[roads[i].nodes[j+2]].lat, nodes[roads[i].nodes[j+2]].lon);
      l3 = euclid_dist(nodes[roads[i].nodes[j+2]].lat, nodes[roads[i].nodes[j+2]].lon,
        nodes[roads[i].nodes[j]].lat, nodes[roads[i].nodes[j]].lon);

      curb = curvature(l1,l2,l3);
      cout << curb << endl;
      if((nodes[roads[i].nodes[j+1]].refs >=2 && nodes[roads[i].nodes[j+2]].refs ==1)
        || curb>thrs || nsize >= nthrs){ //||curvature < threshold
        //break here
        if(nsize < minsize)
          minsize = nsize;
        else if(nsize > maxsize)
          maxsize = nsize;
        cout << "break" << ' ' << nsize << endl;
        //make verbose checking of next node here -> did that
        //out << segid << ' ' << roads[i].id << ' ' << nsize;
        nsize = 2;
        segid++;
      }
      else{
        nsize++;
        coords.push_back(nodes[roads[i].nodes[j+1]].lat);
        coords.push_back(nodes[roads[i].nodes[j+1]].lon);
      }
    }
    coords.push_back(nodes[roads[i].nodes.back().lat);
    coords.push_back(nodes[roads[i].nodes.back().lon);
  }
  cout << "Maxsize: " << maxsize << " Minsize: " << minsize << " Segs: " << segid+1 << endl;
}
