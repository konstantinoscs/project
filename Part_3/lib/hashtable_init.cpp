#include <vector>
#include <string>
#include <limits>

#include "curve.h"
#include "assign_entry.h"
#include "curve_ops.h"
#include "general_hash.h"
#include "minmax.h"
#include "distance_ops.h"
#include "lsh.h"

using namespace std;

int isCentroid(string id, const vector<real_curve*> & centroids){
  for(unsigned int i=0; i<centroids.size(); i++){
    //if it's centroid will return here something != 0
    if(!id.compare(centroids[i]->get_id()))//id==c[i]->id
      return i+1;//to avoid returning value 0 here
  }
  return 0;
}

void init_assign_entries(vector<assign_entry> & entries,
  vector<real_curve> & curves){
  entries.clear();
  for(unsigned int i=0; i<curves.size(); i++){
    assign_entry entry{};
    entry.rcurve = &curves[i];
    entry.centroid = -1;
    entry.distance = -1;
    entries.push_back(std::move(entry));
  }
}

void init_hashtable(int L, int k, vector<assign_entry> & entries,
  int dimension, double  delta,int kvec, int w,
  const vector<real_curve> & curves, int tablesize,
  vector<vector<vector<assign_entry*>>> &Lht){

  vector<real_curve> normalized_curves{};//just temporary to call below function 
  vector<vector<norm_curve>> concat_normalized_curves{};
  int max{};
  Lconcatenate_kcurves(k, L ,curves, dimension, delta,
    concat_normalized_curves, max, normalized_curves);

  hash_curves(concat_normalized_curves, dimension*k*max, Lht,
    tablesize, entries, kvec, w);

}

void find_keys(vector<std::vector<vector<assign_entry*>>> &Lht,
  const vector<real_curve*> & centroids,
  vector<vector<int>> & centroid_keys){
  int k{};
  centroid_keys.clear();
  centroid_keys.resize(centroids.size());
  for(unsigned int i=0; i<Lht.size(); i++){//0<=i<L
    for(unsigned int j=0; j<Lht[i].size(); j++){//0<=j<tablesize
      for(unsigned int z=0; z<Lht[i][j].size(); z++){//every curve in bucket
        k = isCentroid(Lht[i][j][z]->rcurve->get_id(),centroids);
        if(k){//k in [1,c] if its centroid else k=0
          centroid_keys[k-1].push_back(j);
        }
      }
    }
  }
}

void create_mean_keys(int w, int k, int dimension, int tablesize, int L,
  vector<real_curve*> & centroids, int delta,
  vector<vector<int>> & centroid_keys){
  vector<hash_f> hs;
  vector<int> g;
  vector<int> r{};
  int key{} ,max;
  vector<real_curve> realc{};
  for(unsigned int i=0; i<centroids.size(); i++)
    realc.push_back(*centroids[i]);
  vector<vector<norm_curve>> concat_centr{};
  vector<real_curve> temp{};
  Lconcatenate_kcurves(k,1,realc,dimension,delta,concat_centr,
    max,temp);
  init_r(dimension*max*k,r);
  centroid_keys.clear();
  centroid_keys.resize(centroids.size());
  make_hashes(hs, w, dimension, k);
  make_g(hs, g);
  for(unsigned int i=0; i<centroids.size(); i++){
    vector<int> h_results{};
    for (unsigned int j=0; j<g.size(); j++)
      h_results.push_back(hs[g[j]].hash(concat_centr[0][i].as_vector()));
    linear_combination(h_results,r,key,tablesize);
    for(int j=0; j<L; j++)
      centroid_keys[i].push_back(key);
  }
  return ;
}
