#include <leximaxIST_Encoder.h>
#include <leximaxIST_error.h>
#include <vector>

namespace leximaxIST {

    // free clauses in m_soft_clauses and clear vector
    void Encoder::clear_soft_clauses()
    {
        for (Clause *cl : m_soft_clauses)
            delete cl;
        m_soft_clauses.clear();
    }

    void Encoder::clear_sorted_relax()
    {
        for (std::vector<std::vector<int>*> &sorted_relax_vecs : m_sorted_relax_collection) {
            for (size_t i (0); i < sorted_relax_vecs.size(); ++i) {
                delete sorted_relax_vecs[i];
                sorted_relax_vecs[i] = nullptr;
            }
        }
        m_sorted_relax_collection.clear();
    }

    // free clauses in m_constraints and clear vector
    void Encoder::clear_hard_clauses()
    {
        for (Clause *cl : m_constraints)
            delete cl;
        m_constraints.clear();
    }

    // members that are not a user parameter are cleared and set to initial value
    void Encoder::clear()
    {
        // free m_objectives
        for (std::vector<int> *objective : m_objectives)
            delete objective;
        m_objectives.clear();
        // free m_sorted_vecs
        for (std::vector<int> *sorted_vec : m_sorted_vecs)
            delete sorted_vec;
        m_sorted_vecs.clear();
        // clear relaxation variables
        m_all_relax_vars.clear();
        // free sorted_relax_vecs
        clear_sorted_relax();
        clear_hard_clauses();
        clear_soft_clauses();
        m_ub_vec.clear();
        m_solution.clear();
        //m_num_opts = 0;
        m_id_count = 0;
        m_num_objectives = 0;
        m_solver_output = false;
        m_child_pid = 0;
        m_sat = false;
        m_sorting_net_size = 0;
        m_times.clear();
    }
    
    // remove temporary files and free memory
    Encoder::~Encoder()
    {
        if (!m_leave_temporary_files)
            remove_tmp_files();
        clear();
    }

}/* namespace leximaxIST */
