#ifndef LEXIMAX_ENCODER
#define LEXIMAX_ENCODER
#include <assert.h>
#include <string>
#include <iostream> // std::cout, std::cin
#include <vector> // std::vector
#include <utility> // std::pair
#include <forward_list> // std::forward_list
#include <leximax_types.h>
#include <sys/types.h>

class Leximax_encoder {

private:

    bool m_debug;
    size_t m_id_count;
    std::vector<Clause*> m_constraints;
    std::vector<Clause*> m_soft_clauses;
    std::vector<std::vector<LINT>*> m_objectives;
    int m_num_objectives;
    std::vector<std::vector<LINT>*> m_sorted_vecs;
    std::vector<std::vector<LINT>*> m_sorted_relax_vecs;
    std::vector<std::vector<std::vector<LINT>*>>  m_sorted_relax_collection;
    std::vector<std::forward_list<LINT>> m_all_relax_vars; // relax_vars of each iteration
    std::string m_solver_command;
    std::string m_solver_format;
    std::string m_lp_solver;
    std::string m_valid_lp_solvers[6];
    std::string m_file_name;
    std::string m_err_file;
    bool m_solver_output; // is there an output file to read?
    pid_t m_child_pid;
    double m_timeout; // timeout for signal handling in milliseconds
    bool m_leave_temporary_files;
    bool m_sat;
    std::string m_multiplication_string;
    std::vector<LINT> m_solution;
    size_t m_sorting_net_size; // size of largest sorting network
    
public:    

    /* a clause is a disjunction of literals;
     * a literal is a variable or a negation of a variable;
     * each variable is represented as a positive integer (LINT);
     * LINT is the largest integer type of your machine (see leximax_types.h);
     * a clause is represented as a vector of literals (integers)
     * constraints is a collection of clauses;
     * each entry of constraints contains a clause, stored as a vector;
     * each entry of the clause vector is a literal of that clause.
     * objective_functions is a collection of objective functions;
     * each objective function is a vector of soft clauses;*/
    // each objective function is the sum of its falsified soft clauses.
    Leximax_encoder(const std::vector<std::vector<LINT>> &constraints, const std::vector<std::vector<std::vector<LINT>>> &objective_functions); 
    // TODO: add checks if constraints and/or obj functions is empty
    ~Leximax_encoder();
    
    // returns 0 if all want well, -1 otherwise
    int solve();
    
    bool get_sat() const;
    
    size_t get_sorting_net_size() const;
    
    /* if the problem is satisfiable, then m_solution is a satisfying assignment;
     * each entry i of m_solution is +i if variable i is true and -i otherwise;
     * if the problem is not satisfiable, m_solution is empty.*/
    const std::vector<LINT>& get_solution() const;
    
    // empty if unsat
    std::vector<LINT> get_objective_vector() const;
    
    void set_solver_command(const std::string &command);
    
    int set_solver_format(const std::string &format);
    
    void set_err_file(const std::string &name);
    
    int set_lp_solver(const std::string &lp_solver);
    
    void set_leave_temporary_files(bool val);
    
    void set_multiplication_string(const std::string &str);
    
    void terminate(int signum);
    
private:
    
    // getters.cpp
    
    std::vector<LINT> get_objective_vector(const std::vector<LINT> &assignment) const;
    
    // constructors.cpp
    
    void add_soft_clause(const std::vector<LINT> &lits);
    
    void add_hard_clause(const std::vector<LINT> &lits);
    
    void update_id_count(const std::vector<LINT> &clause);
    
    // destructor.cpp
    
    void clear_soft_clauses();
    
    void clear_sorted_relax();
    
    void clear_hard_clauses();
    
    // sorting_net.cpp
    
    void encode_max(LINT var_out_max, LINT var_in1, LINT var_in2);
    
    void encode_min(LINT var_out_min, LINT var_in1, LINT var_in2);
    
    void insert_comparator(LINT el1, LINT el2, std::vector<LINT> *objective, SNET &sorting_network);
    
    void odd_even_merge(std::pair<std::pair<LINT,LINT>,LINT> seq1, std::pair<std::pair<LINT,LINT>,LINT> seq2, std::vector<LINT> *objective, SNET &sorting_network);
    
    void encode_network(std::pair<LINT,LINT> elems_to_sort, std::vector<LINT> *objective, SNET &sorting_network);
    
    void delete_snet(SNET &sorting_network);
    
    // encoding.cpp
    
    void encode_sorted();
    
    size_t largest_obj();
    
    void generate_soft_clauses(int i);
    
    void all_subsets(std::forward_list<LINT> set, int i, std::vector<LINT> &clause_vec);
    
    void at_most(std::forward_list<LINT> &set, int i);
    
    void encode_relaxation(int i);
    
    void componentwise_OR(int i);
    
    // solver_call.cpp
    
    void reset_file_name();

    int split_solver_command(const std::string &command, std::vector<std::string> &command_split);
    
    int call_solver(const std::string &input_filename);
    
    int read_solver_output(std::vector<LINT> &model);
    
    int external_solve(int i);
    
    int write_solver_input(int i);
    
    int write_lp_file(int i);
    
    int write_opb_file(int i);
    
    int write_wcnf_file(int i);
    
    int read_cplex_output(std::vector<LINT> &model);
    
    int read_gurobi_output(std::vector<LINT> &model);
    
    int read_glpk_output(std::vector<LINT> &model);
    
    int read_lpsolve_output(std::vector<LINT> &model);
    
    int read_scip_output(std::vector<LINT> &model);
    
    int read_cbc_output(std::vector<LINT> &model);
    
    // printing.cpp
    
    void print_error_msg(const std::string &msg) const;
    
    void print_waitpid_error(const std::string errno_str) const;
    
    void print_clause(std::ostream &output, Clause * const cl);
    
    void print_clauses(std::ostream &output, const std::vector<Clause*> &clauses, size_t weight);
    
    void print_atmost_lp(int i, std::ostream &output);
    
    void print_lpconstraint(Clause * const cl, std::ostream &output);
    
    void print_sum_equals_lp(int i, std::ostream &output);
    
    void print_atmost_pb(int i, std::ostream &output);
    
    void print_pbconstraint(Clause * const cl, std::ostream &output);
    
    void print_sum_equals_pb(int i, std::ostream &output);

};
    
#endif
