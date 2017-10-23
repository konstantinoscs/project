#include <vector>

#include "hash_f.h"
#include "general_hash.h"
#include "rand_utils.h"

using namespace std;

//makes k hash functions and puts them in hs
void make_hashes(vector<hash_f> & hs, int w, int v_size, int k){
  for(int i =0; i<k; i++)
    hs.push_back(std::move(hash_f(i, w, v_size)));
}

//choose k h_i randomly, the indexes of the hash functions are
//put in an int array g
//g and hs have the same size!
void make_g(const vector<hash_f> & hs, vector<int> g){
  for(size_t i=0; i<hs.size(); i++)
    g.push_back(int_uniform_rand(0, hs.size()-1));
}

void lsh_curve_hashing(const vector<int> & concat_norm_points, vector<int> & r,
  vector<entry> *ht, int tablesize, vector<real_curve*> & curves,
  int curve_index, vector<real_curve> & normalized_curves, int *g,
  std::vector<hash_f> & hs){

  int key{};
  entry temp{};
  vector<int> h_results;

  //remember: g and hs have the same size!
  for (size_t i=0; hs.size(); i++){
    h_results.push_back(hs[g[i]].hash(concat_norm_points));
  }

  linear_combination(h_results,r,key,tablesize);
  temp.rcurve = curves[curve_index];
  temp.gcurve = &normalized_curves[curve_index];
  ht[key].push_back(std::move(temp));
}
