// parse_and_fill_tree.h
#ifndef PARSE_AND_FILL_TREE_H
#define PARSE_AND_FILL_TREE_H

#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TObject.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iosfwd>   // per std::istream forward-declare
class TTree;
struct RunInfo;
struct EventData;

// core riutilizzabile
void parse_and_fill_tree_core(std::istream& input,
                              TTree&        tree,
                              RunInfo&      runInfo,
                              EventData&    event);

// vecchia interfaccia .sim -> .root
void parse_and_fill_tree(const char* inputFile, const char* outputFile);

#endif
