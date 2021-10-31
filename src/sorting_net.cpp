#include <leximaxIST_Encoder.h>
#include <leximaxIST_printing.h>
#include <leximaxIST_types.h>
#include <utility>
#include <vector>

namespace leximaxIST {

    /* a sorting network is a vector with size equal to the number of wires.
    * Entry i contains the last comparator (in the construction of the sorting network) that connects to wire i.
    * The comparator is represented as a pair. The first component is the other wire j that the comparator connects to.
    * The second component is the variable representing
    * the smallest of the outputs, if i < j
    * the greatest of the outputs, if i > j.*/

    void Encoder::encode_max(int var_out_max, int var_in1, int var_in2)
    {
        // encode var_out_max is equivalent to var_in1 OR var_in2
        add_hard_clause(-var_out_max, var_in1, var_in2);
        add_hard_clause(var_out_max, -var_in1);
        add_hard_clause(var_out_max, -var_in2);
    }

    void Encoder::encode_min(int var_out_min, int var_in1, int var_in2)
    {
        // encode var_out_min is equivalent to var_in1 AND var_in2
        add_hard_clause(-var_out_min, var_in1);
        add_hard_clause(-var_out_min, var_in2);
        add_hard_clause(var_out_min, -var_in1, -var_in2);
    }

    void Encoder::insert_comparator(int el1, int el2, const std::vector<int> *objective, SNET &sorting_network)
    {
        //std::cerr << "Inserting comparator between wires " << el1 << " and " << el2 << std::endl;
        m_sorting_net_size++;
        // if the entry is empty, then it is the first comparator for that wire
        int var_in1 = (sorting_network[el1].first == -1) ? objective->at(el1) : sorting_network[el1].second;
        int var_in2 = (sorting_network[el2].first == -1) ? objective->at(el2) : sorting_network[el2].second;
        int var_out_min = fresh();
        int var_out_max = fresh();
        // encode outputs, if el1 > el2 then el1 is the largest, that is, the or. Otherwise, el1 is the smallest, i.e. the and.
        encode_max(var_out_max, var_in1, var_in2);
        encode_min(var_out_min, var_in1, var_in2);
        std::pair<int,int> comp1 (el2, 0);
        std::pair<int,int> comp2 (el1, 0);
        if(el1 > el2)
            comp1.second = var_out_max;
        else
            comp1.second = var_out_min;
        sorting_network[el1] = comp1;
        if(el2 > el1)
            comp2.second = var_out_max;
        else
            comp2.second = var_out_min;
        sorting_network[el2] = comp2;
    }

    int ceiling_of_half(int number)
    {
        // floor
        int result = number/2;
        // if number is odd then add 1
        if(number % 2 != 0)
            result++;
        return result;
    }

    void Encoder::odd_even_merge(std::pair<std::pair<int,int>,int> seq1, std::pair<std::pair<int,int>,int> seq2, const std::vector<int> *objective, SNET &sorting_network)
    {
        int el1;
        int el2;
        int size1 = seq1.first.second;
        int size2 = seq2.first.second;
        if(size1 == 0 || size2 == 0){
            // nothing to merge
        }
        else if(size1 == 1 && size2 == 1){
            // merge two elements with a single comparator.
            el1 = seq1.first.first;
            el2 = seq2.first.first;
            insert_comparator(el1, el2, objective, sorting_network);
        }
        else {
            // merge odd subsequences
            // size of odd subsequence is the ceiling of half of the size of the original sequence
            int offset1 = seq1.second;
            int offset2 = seq2.second;
            int first1 = seq1.first.first;
            int first2 = seq2.first.first;
            std::pair<int,int> p1(first1, ceiling_of_half(size1));
            std::pair<std::pair<int,int>,int> odd1(p1, 2*offset1);
            std::pair<int,int> p2(first2, ceiling_of_half(size2));
            std::pair<std::pair<int,int>,int> odd2(p2, 2*offset2);
            odd_even_merge(odd1, odd2, objective, sorting_network);
            // merge even subsequences
            // size of even subsequence is the floor of half of the size of the original sequence
            p1 = std::make_pair(first1 + offset1, size1/2);
            std::pair<std::pair<int,int>,int> even1(p1, 2*offset1);
            p2 = std::make_pair(first2 + offset2, size2/2);
            std::pair<std::pair<int,int>,int> even2(p2, 2*offset2);
            odd_even_merge(even1, even2, objective, sorting_network);
            // comparison-interchange
            for(int i{2}; i <= size1; i = i + 2){
                if(i == size1){
                    // connect last of seq1 to first element of seq2
                    el1 = first1 + offset1*(size1-1);
                    el2 = first2;
                    insert_comparator(el1, el2, objective, sorting_network);
                }
                else{
                    // connect i-th of seq1 with i+1-th of seq1
                    el1 = first1 + offset1*(i-1);
                    el2 = el1 + offset1;
                    insert_comparator(el1, el2, objective, sorting_network);
                }
            }
            int init = (size1 % 2 == 0) ? 2 : 1 ;
            for(int i{init}; i < size2; i = i + 2){
                    // connect i-th of seq2 with i+1-th of seq2
                    el1 = first2 + offset2*(i-1);
                    el2 = el1 + offset2;
                    insert_comparator(el1, el2, objective, sorting_network);            
            }
        }
    }

    void Encoder::encode_network(const std::pair<int,int> elems_to_sort, const std::vector<int> *objective, SNET &sorting_network)
    {
        int size = elems_to_sort.second;
        int first_elem = elems_to_sort.first;
        if(size == 1){
            // do nothing - a single element is already sorted.
        }
        if(size > 1){
            int m = size/2;
            int n = size - m;
            std::pair<int,int> split1(first_elem, m);
            std::pair<int,int> split2(first_elem + m, n);
            // recursively sort the first m elements and the last n elements
            encode_network(split1, objective, sorting_network);
            encode_network(split2, objective, sorting_network);
            // merge the sorted m elements and the sorted n elements
            std::pair<std::pair<int,int>,int> seq1(split1,1);
            std::pair<std::pair<int,int>,int> seq2(split2,1);
            odd_even_merge(seq1, seq2, objective, sorting_network);
        }
    }
    
    /* Create a sorting network to sort the obj_vars
     * Merge this sorting network with the old sorting network - the outputs are initially in sorted_vec
     * sorted_vec is changed and set to the outputs of the new (larger) sorting network.
     */
    void Encoder::merge_core_guided(int obj_index, const std::vector<int> &obj_vars)
    {
        std::vector<int> *sorted_vec = m_sorted_vecs.at(obj_index);
        const size_t old_size (sorted_vec->size());
        SNET sort_new_vars(obj_vars.size(), {-1,-1});
        // Create a sorting network to sort the obj_vars
        const std::pair<int,int> elems_to_sort(0, obj_vars.size());
        encode_network(elems_to_sort, &obj_vars, sort_new_vars);
        
        // Merge
        SNET new_sort_net(old_size + obj_vars.size(), {-1,-1});
        // the first old_size entries are equal to the old sorting network
        for (size_t i (0); i < old_size; ++i) {
            /* the first of the pair is the wire that is connected to i, through the last comparator
             * in this case we don't know, so we put 0.
             * we can put any value as long as it is different from -1.
             * -1 is used for the case there is no comparator connecting wire i.
             */
            new_sort_net.at(i).first = 0;
            // the second of the pair is the variable associated with the output of the last comparator of wire i.
            new_sort_net.at(i).second = sorted_vec->at(i);
        }
        // the last obj_vars.size() entries are equal to sort_new_vars
        for (size_t i (old_size); i < old_size + obj_vars.size(); ++i) {
            new_sort_net.at(i) = sort_new_vars.at(i - old_size);
        }
        const std::pair<int, int> p1 (0, old_size);
        const std::pair<int, int> p2 (old_size - 1, obj_vars.size());
        const std::pair<std::pair<int, int>, int> seq1 (p1, 1); // ((first wire, number of elements), offset)
        const std::pair<std::pair<int, int>, int> seq2 (p2, 1);
        // Since the sorting network has comparators connecting all wires, hence the nullptr. It is not used.
        odd_even_merge(seq1, seq2, nullptr, new_sort_net);
        // set sorted_vec to the outputs of new_sort_net
        sorted_vec->resize(old_size + obj_vars.size());
        for (size_t i (0); i < old_size + obj_vars.size(); ++i)
            sorted_vec->at(i) = new_sort_net.at(i).second;
    }
}/* namespace leximaxIST */
