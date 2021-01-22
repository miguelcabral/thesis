#include <Leximax_encoder.h>

void Leximax_encoder::set_solver_command(const std::string &command)
{
    m_solver_command = command;
}

int Leximax_encoder::set_solver_format(const std::string &format)
{
    m_solver_format = format;
    if (format != "wcnf" && format != "opb" && format != "lp") {
        std::string msg ("The external solver format entered: '" + format + "' is not valid\n");
        msg += "Valid external solver formats: 'wcnf' 'opb' 'lp'";
        print_error_msg(msg);
        return -1;
    }
    return 0;    
}

int Leximax_encoder::set_lp_solver(const std::string &lp_solver)
{
    m_lp_solver = lp_solver;
    bool found (false);
    for (std::string &valid_lp_solver : m_valid_lp_solvers)
        if (lp_solver == valid_lp_solver)
            found = true;
    if (!found) {
        std::string msg = "The lp solver name entered: '" + lp_solver + "' is not valid\n";
        msg += "Valid lp solvers: ";
        for (std::string &valid_lp_solver : m_valid_lp_solvers)
            msg += valid_lp_solver + ' ';
        print_error_msg(msg);
        return -1;
    }
    return 0;
}

void Leximax_encoder::set_leave_temporary_files(bool val) { m_leave_temporary_files = val; }

void Leximax_encoder::set_multiplication_string(const std::string &str) { m_multiplication_string = str; }

void Leximax_encoder::set_err_file(const std::string &name) { m_err_file = name; }