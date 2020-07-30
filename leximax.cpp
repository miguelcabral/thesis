#include <string>
#include <iostream> // std::cout, std::cin
#include <vector> // std::vector
#include <utility> // std::pair
#include "old_packup/basic_clause.hh"
#include "old_packup/basic_clset.hh"
#include "old_packup/basic_types.h"
#include "old_packup/clause_utils.hh"
#include "old_packup/cl_registry.hh"
#include "old_packup/cl_globals.hh"
#include "old_packup/cl_types.hh"
#include "old_packup/ReadCNF.hh"
#include "sorting_net.h"


void encode_fresh(ReadCNF &hard, BasicClause *cl, LINT fresh_var)
{
    // fresh_var implies cl
    std::vector<LINT> lits;
    lits.push_back(-fresh_var);
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        lits.push_back(*it);
    }
    hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    // not fresh_var implies not cl
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        lits.clear();
        lits.push_back(-*it);
        lits.push_back(fresh_var);
        hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
    }  
}

void print_clause(BasicClause *cl)
{
    for(Literator it = cl->begin(); it != cl->end(); ++it){
        std::cout << *it << " ";
    };
    std::cout << "0" << std::endl;
}

void print_cnf(ReadCNF &hard, LINT id_count)
{
    std::vector<BasicClause*> cls = hard.get_clause_vector();
    std::cout << "p cnf " << id_count << " " << cls.size() << std::endl;
    for(LINT i{0}; i < cls.size(); ++i){
        print_clause(cls.at(i));
    }
}

int main(int argc, char *argv[])
{
    // read input problem: hard.cnf f_1.cnf f_2.cnf ...
    int num_objectives = argc-2;
    gzFile in = gzopen(argv[1], "rb");
    if (in == Z_NULL) {
       exit(0);
    }
    ReadCNF hard(in);
    hard.read();
    gzclose(in);
    std::vector<ReadCNF*> read_objectives;
    for(int i{2}; i < argc; ++i){
        in = gzopen(argv[i], "rb");
        if (in == Z_NULL) {
            exit(0);
        }
        ReadCNF *obj = new ReadCNF(in);
        obj->read();
        gzclose(in);
        read_objectives.push_back(obj);
    }
    std::vector<std::vector<LINT>*> objectives(num_objectives);
    LINT id_count{ hard.get_max_id() };
    // convert soft clauses to objective function.
    for(int i{0}; i < num_objectives; ++i){
        ReadCNF *obj = read_objectives[i];
        std::vector<BasicClause*> cls = obj->get_clause_vector();
        std::vector<LINT> *obj_conv = new std::vector<LINT>();
        for(LINT j{0}; j < cls.size(); ++j){
            BasicClause *cl = cls[j];
            if(cl->size() > 1 || *(cl->begin()) < 0){
                LINT fresh_var = id_count + 1;
                id_count++;
                // encode fresh_var
                encode_fresh(hard, cl, fresh_var);
                obj_conv->push_back(fresh_var);
            }
            else
                obj_conv->push_back(*(cl->begin()));
        }
        objectives[i] = obj_conv;        
    }
    // encode with odd even merge sorting network
    std::vector<std::pair<LINT, LINT>> sorted_vecs(num_objectives);
    for(int i{0}; i < num_objectives; ++i){   
        std::vector<LINT> *objective = objectives[i];
        LINT num_terms = objective->size();
        sorted_vecs[i]= std::make_pair(id_count + 1, id_count + num_terms);
        id_count += num_terms;
        SNET sorting_network(num_terms, nullptr); // sorting_network is initialized to a vector of nullptrs
        // elems_to_sort is represented by a pair (first element, number of elements).
        std::pair<LINT,LINT> elems_to_sort(0,num_terms);
        encode_network(hard, elems_to_sort, id_count, objective, sorting_network);
        // relate outputs of sorting_network with sorted_vec variables
        for(LINT j{0}; j < num_terms; j++){
            LINT output_j = sorting_network[j]->second;
            LINT o = sorted_vecs[i].first + j;
            // encode o is equivalent to output_j
            std::vector<LINT> lits;
            lits.push_back(-o);
            lits.push_back(output_j);
            hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
            lits.clear();
            lits.push_back(o);
            lits.push_back(-output_j);
            hard.get_clause_vector().push_back(hard.get_clauses().create_clause(lits));
        }

    }
    // check if sorting_network is working: print clauses and send to sat solver.
    print_cnf(hard, id_count);
    return 0;
}
