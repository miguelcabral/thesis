#ifndef LEXIMAXIST_ENCODER
#define LEXIMAXIST_ENCODER
#include <leximaxIST_types.h>
#include <IpasirWrap.h>
#include <string> // std::string
#include <vector> // std::vector
#include <utility> // std::pair
#include <list> // std::list
#include <sys/types.h> // pid_t


namespace leximaxIST
{

    class Encoder {
        /* TODO: check if pointers can be removed without affecting performance
         * I think we can do that since vectors are about 24 bytes
         * (because they only store the pointer to the array, not the entire array)
         * and when there is a copy, only the 24 bytes are copied, not the array.
         * ADVANTAGES: The code becomes simpler and safer (no memory leaks guaranteed)
         * DISADVANTAGES: Maybe it gets slower, but I do not think so
         */

    private:

        int m_verbosity; // 0: nothing, 1: general solving info, 2: everything including encoding
        int m_id_count;
        std::vector<Clause*> m_constraints; // TODO: remove pointers?
        std::vector<Clause*> m_soft_clauses; // TODO: remove pointers?
        std::vector<std::vector<int>*> m_objectives; // TODO: remove pointers?
        int m_num_objectives;
        std::vector<std::vector<int>*> m_sorted_vecs; // TODO: remove pointers?
        std::vector<std::vector<std::vector<int>*>>  m_sorted_relax_collection; // TODO: remove pointers?
        std::vector<std::list<int>> m_all_relax_vars; // relax_vars of each iteration
        std::string m_opt_solver_cmd; // for external call to optimisation solver
        std::string m_formalism;
        std::string m_lp_solver;
        std::string m_valid_lp_solvers[6];
        std::string m_file_name;
        bool m_solver_output; // is there an output file to read?
        pid_t m_child_pid;
        double m_timeout; // timeout for signal handling in milliseconds
        bool m_leave_temporary_files;
        bool m_simplify_last; // if true the algorithm does not use the sorting networks in the last iteration
        bool m_sat;
        std::string m_sat_solver_cmd; // for external call to sat solver
        int m_ub_encoding; // 0: do nothing, 1: get upper bound with sat call, 2: get upper bound with MSS, 3: get upper bound with MaxSAT
        // the next one is usefull if computation is stoped and you get an intermediate solution
        // you want to know which values of the objective vector are in theory guaranteed to be optimal
        //int m_num_opts; // number of optimal values found: 0 = none; 1 = first maximum is optimal; 2 = first and second; ...
        std::string m_multiplication_string;
        std::vector<int> m_solution;
        size_t m_sorting_net_size; // size of largest sorting network
        //std::vector<double> m_times; // time of each step of solving (only external solver times)
        
    public:    

        Encoder(); 

        ~Encoder();
        
        // returns 0 if all want well, -1 otherwise
        int solve();
                
        bool get_sat() const;
        
        //int get_num_opts() const;
        
        const std::vector<double>& get_times() const;
        
        /* if the problem is satisfiable, then m_solution is a satisfying assignment;
        * each entry i of m_solution is +i if variable i is true and -i otherwise;
        * if the problem is not satisfiable, m_solution is empty.*/
        const std::vector<int>& get_solution() const;
        
        // empty if unsat; not a const reference because it is not a member variable
        std::vector<int> get_objective_vector() const;
        
        int set_problem(const std::vector<std::vector<int>> &constraints, const std::vector<std::vector<std::vector<int>>> &objective_functions);
        
        void set_simplify_last(bool val);
        
        int set_ub_encoding(int val);
        
        void set_timeout(double val); // for terminate function
        
        void set_opt_solver_cmd(const std::string &command);
        
        void set_sat_solver_cmd(const std::string &command);
        
        int set_formalism(const std::string &format);
        
        int set_verbosity(int v); // if value is invalid return -1 and leave m_verbosity as it was
        
        int set_lp_solver(const std::string &lp_solver);
        
        void set_leave_temporary_files(bool val); // TODO: change temporary to tmp
        
        void set_multiplication_string(const std::string &str);
        
        int terminate(); // kill external solver and read approximate solution
        
        void clear(); // frees memory, and sets internal parameters to their initial value
        
    private:
        
        // setters.cpp
        
        int fresh();
        
        void reset_file_name();
        
        void update_id_count(const Clause &clause);
        
        // getters.cpp
        
        std::vector<int> get_objective_vector(const std::vector<int> &assignment) const;
        
        // constructors.cpp
        
        int add_hard_clause(const Clause &c);
        
        // uses move constructor; if Clause is small don't use
        int add_hard_clause(Clause &&cl);
        
        int add_soft_clause(const Clause &c);
        
        // uses move constructor; if Clause is small don't use
        int add_soft_clause(Clause &&cl); // for move constructor
        
        // destructor.cpp
        
        void clear_soft_clauses();
        
        void clear_sorted_relax();
        
        void clear_hard_clauses();
        
        // sorting_net.cpp
        
        void encode_max(int var_out_max, int var_in1, int var_in2);
        
        void encode_min(int var_out_min, int var_in1, int var_in2);
        
        void insert_comparator(int el1, int el2, const std::vector<int> *objective, SNET &sorting_network);
        
        void odd_even_merge(std::pair<std::pair<int,int>,int> seq1, std::pair<std::pair<int,int>,int> seq2, const std::vector<int> *objective, SNET &sorting_network);
        
        void encode_network(std::pair<int,int> elems_to_sort, const std::vector<int> *objective, SNET &sorting_network);
        
        //void delete_snet(SNET &sorting_network);
        
        // encoding.cpp
        
        void encode_sorted();
        
        size_t largest_obj();
        
        void generate_soft_clauses(int i);
        
        void all_subsets(std::list<int> set, int i, Clause &clause);
        
        void at_most(const std::list<int> &set, int i);
        
        void encode_relaxation(int i);
        
        void componentwise_OR(int i);
        
        void encode_upper_bound(int i, std::vector<int> &old_obj_vec);
        
        void debug_print_all(const std::vector<std::vector<int>> &true_ys, const std::vector<int> &y_vector);
        
        // solver_call.cpp
        
        void mss_add_falsified (IpasirWrap &solver, std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector) const;
        
        int mss_solve();
        
        int sat_solve();
        
        int calculate_upper_bound();
        
        void remove_tmp_files() const;

        int split_command(const std::string &command, std::vector<std::string> &command_split);
        
        int call_solver(const std::string &solver_type);
        
        int read_solver_output(std::vector<int> &model);
        
        int external_solve(int i);
        
        int write_solver_input(int i);
        
        int write_cnf_file(int i);
        
        int write_lp_file(int i);
        
        int write_opb_file(int i);
        
        int write_wcnf_file(int i);
        
        int read_sat_output(std::vector<int> &model);
        
        int read_cplex_output(std::vector<int> &model);
        
        int read_gurobi_output(std::vector<int> &model);
        
        int read_glpk_output(std::vector<int> &model);
        
        int read_lpsolve_output(std::vector<int> &model);
        
        int read_scip_output(std::vector<int> &model);
        
        int read_cbc_output(std::vector<int> &model);
        
        // printing.cpp
        
        void print_soft_clauses(int i) const;
        
        void print_sorted_vec (int i) const;
        
        void print_obj_func(int i) const;
        
        void print_snet_size(int i) const;
        
        void print_mss_todo(const std::vector<std::vector<int>> &todo_vec) const;
        
        void print_obj_vector(const std::vector<int> &obj_vec) const;
        
        void print_obj_vector() const; // this one computes the obj_vector
        
        void print_waitpid_error(const std::string &errno_str) const;
        
        void print_clause(std::ostream &output, const Clause *cl, const std::string &leadingStr = "") const;
        
        void print_wcnf_clauses(std::ostream &output, const std::vector<Clause*> &clauses, size_t weight) const;
        
        void print_atmost_lp(int i, std::ostream &output) const;
        
        void print_lp_constraint(const Clause *cl, std::ostream &output) const;
        
        void print_sum_equals_lp(int i, std::ostream &output) const;
        
        void print_atmost_pb(int i, std::ostream &output) const;
        
        void print_pb_constraint(const Clause *cl, std::ostream &output) const;
        
        void print_sum_equals_pb(int i, std::ostream &output) const;

    };

}

#endif