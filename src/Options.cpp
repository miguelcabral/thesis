#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include "Options.hh"
using std::cerr;
using std::endl;

Options::Options()
: m_help (0)
, m_verbosity (0)
, m_simplify_last (0)
, m_solve ("core-merge")
, m_input_file_name ("")
, m_disjoint_cores (0)
, m_approx ("none")
, m_timeout (86400)
, m_mss_tol (0)
, m_mss_add_cls (1)
, m_mss_incr (0)
, m_gia_pareto (0)
, m_gia_incr (0)
{
    // TODO: add the descriptions of the Options
}

bool Options::parse(int argc,char **argv) {
    bool return_value = true;

    static struct option long_options[] = {
         {"help", no_argument,    &m_help, 1}
       ,{"external-solver", required_argument,  0, 500}
       ,{"multiplication-string",  required_argument,  0, 501}
       ,{"leave-temporary-files",  no_argument,  &m_leave_temporary_files, 1}
       ,{"pbo", no_argument,  &m_pbo, 1}
       ,{0, 0, 0, 0}
             };

    int c;
    while (1) {
       /* getopt_long stores the option index here. */
       int option_index = 0;
       c = getopt_long(argc, argv, "h", long_options, &option_index);
       opterr = 0;
       /* Detect the end of the options. */
       if (c == -1) break;
       switch (c) {
            case 0: if (long_options[option_index].flag != 0) break;
                    else return false;
            case 'h': m_help     = 1; break;
            case 500: m_solver = optarg; break;
            case 501: m_multiplication_string = optarg; break;
            case '?':
             if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else
               fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
             return_value = false;
             break;
           default:
               return false;
        }
    }

    if (!m_help) {
        if (optind >= argc) {
            cerr << "cnf files expected" << endl;
            return_value = false;
        } else {
            while (optind < argc) {
                // store name of input files
                m_input_files.push_back(argv[optind++]);
                m_num_objectives++;
            }
        }
    }

    if (optind < argc) {
        cerr << "WARNING: Unprocessed options at the end." << endl;
    }
    
    if (m_num_objectives == 0) {
        cerr << "objective function(s) expected" << endl;
        return_value = false;
    }
    
    return return_value;
}

