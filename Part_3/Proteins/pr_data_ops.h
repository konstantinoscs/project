#ifndef PR_DATA_OPS_H
#define PR_DATA_OPS_H

#include <string>
#include <vector>

#include "../lib/curve.h"

bool parse_arguments(int argc, char **argv, std::string & input);

bool parse_input(std::string config_s, int & numConform, int & N, std::vector<real_curve> & proteins);

void write_results(std::string out_s, std::vector<std::vector<std::string>> & best_assignment,double Stotal);

#endif
