// ConnectivityManager.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////
  

#include "Macros.h"
#include "ConnectivityManager.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// RandomFullPreConnectivity function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//! RandomFullPreConnectivity constructor with script file
RandomFullPreConnectivity::RandomFullPreConnectivity(std::ifstream & is, const Size & size_gp_postnrn)
{
    post_neuron_list_ = std::vector<bool>(size_gp_postnrn, false);
    std::string test;
    double proba_connection;
    READ_FROM_FILE(is, proba_connection, "proba_connection", "RandomFullPreConnectivity")
    for (std::vector<bool>::iterator i = post_neuron_list_.begin();
            i != post_neuron_list_.end();
            ++i) {
        if (RandomGenerator::dran(1.) < proba_connection)
            (*i) = true;
    }
}

/////////////////////////////////////////////////
//! do_connect checks if post neuron in the list
bool RandomFullPreConnectivity::do_connect(const Size & n_prenrn, const Size & n_postnrn)
{
    if (n_postnrn < post_neuron_list_.size())
        return (post_neuron_list_[n_postnrn]);
    else
        throw ConfigError("ListConnectivity: unexpected discripency between the max index of the connection list and the postneuron group");
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// RandomFixedPreConnectivity function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//! RandomFixedPreConnectivity constructor with script file
/*! each postneuron will have exactly nb_pre_connection preconnections
 */
RandomFixedPreConnectivity::RandomFixedPreConnectivity(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn)
{
    con_list_ = std::vector<std::vector<bool> >(size_gp_prenrn, std::vector<bool>(size_gp_postnrn, false));
    std::string test;
    Size n1, rank_in_left_combinations, n2, counter, nb_pre_connection;
    READ_FROM_FILE(is, nb_pre_connection, "nb_pre_connection", "RandomFixedPreConnectivity")
    for (n2 = 0; n2 < size_gp_postnrn; ++n2) {
        counter = 0; // number of effective preconnections made so far
        while (counter < nb_pre_connection) {
            rank_in_left_combinations = RandomGenerator::iran(size_gp_prenrn - counter - 1);
            n1 = 0;
            while ((n1 == n2) || (rank_in_left_combinations > 0) || (con_list_[n1][n2] == true)) {
                if ((n1 != n2) && (con_list_[n1][n2] == false))
                    --rank_in_left_combinations;
                ++n1;
            }
            con_list_[n1][n2] = true;
            ++counter;
        }
    }
}

/////////////////////////////////////////////////
//! checks if connection is in the list
bool RandomFixedPreConnectivity::do_connect(const Size & n_prenrn, const Size & n_postnrn)
{
    if ((n_prenrn < con_list_.size()) && (n_postnrn < con_list_[n_prenrn].size()))
        return (con_list_[n_prenrn][n_postnrn]);
    else
        throw ConfigError("ListConnectivity: unexpected discripency between the max indexes of the connection list and the groups to connect");
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// FullByBlockConnectivity function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//! constructor with script file
/*! num_subgp_pre_ must be in [0 .. num_subgp_pre_ - 1] but must be in [1 .. num_subgp_pre_] in the script file
 * same for num_subgp_post_
 */
FullByBlockConnectivity::FullByBlockConnectivity(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn)
{
    std::string test;
    READ_FROM_FILE(is, num_subgp_pre_, "pre", "FullByBlockConnectivity")
    READ_FROM_FILE(is, nb_division_pre_, "on", "FullByBlockConnectivity")
    READ_FROM_FILE(is, num_subgp_post_, "post", "FullByBlockConnectivity")
    READ_FROM_FILE(is, nb_division_post_, "on", "FullByBlockConnectivity")
    --num_subgp_pre_;
    --num_subgp_post_;
    if ((num_subgp_pre_ < 0) || (num_subgp_pre_ >= nb_division_pre_)) throw ConfigError("FullByBlockConnectivity: # of subpool negative or bigger than number of divisions for preneurons minus 1");
    if ((num_subgp_post_ < 0) || (num_subgp_post_ >= nb_division_post_)) throw ConfigError("FullByBlockConnectivity: # of subpool negative or bigger than number of divisions for postneurons minus 1");
    size_gp_prenrn_ = size_gp_prenrn;
    size_gp_postnrn_ = size_gp_postnrn;
}

/*! check if n_prenrn is in [ num_subgp_pre_/nb_division_pre_ , (num_subgp_pre_+1)/nb_division_pre_ )
 * same for n_postnrn
 */
bool FullByBlockConnectivity::do_connect(const Size & n_prenrn, const Size & n_postnrn)
{
    if ((n_prenrn*nb_division_pre_ >= num_subgp_pre_*size_gp_prenrn_) && (n_prenrn*nb_division_pre_ < (num_subgp_pre_ + 1)*size_gp_prenrn_)
            && (n_postnrn*nb_division_post_ >= num_subgp_post_*size_gp_postnrn_) && (n_postnrn*nb_division_post_ < (num_subgp_post_ + 1)*size_gp_postnrn_))
        return true;
    else
        return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// RandomByBlockConnectivity function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//! constructor with script file
/*! num_subgp_pre_ must be in [0 .. num_subgp_pre_ - 1] here but the corresponding value must be in [1 .. num_subgp_pre_] in the script file
 *same for num_subgp_post_
 * as FullByBlockConnectivity but proba connection in addition
 */
RandomByBlockConnectivity::RandomByBlockConnectivity(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn)
{
    std::string test;
    READ_FROM_FILE(is, num_subgp_pre_, "pre", "RandomByBlockConnectivity")
    READ_FROM_FILE(is, nb_division_pre_, "on", "RandomByBlockConnectivity")
    READ_FROM_FILE(is, num_subgp_post_, "post", "RandomByBlockConnectivity")
    READ_FROM_FILE(is, nb_division_post_, "on", "RandomByBlockConnectivity")
    READ_FROM_FILE(is, proba_connection_, "proba_connection", "RandomByBlockConnectivity")
    --num_subgp_pre_;
    --num_subgp_post_;
    if ((num_subgp_pre_ < 0) || (num_subgp_pre_ >= nb_division_pre_)) throw ConfigError("FullByBlockConnectivity: # of subpool negative or bigger than number of divisions for preneurons minus 1");
    if ((num_subgp_post_ < 0) || (num_subgp_post_ >= nb_division_post_)) throw ConfigError("FullByBlockConnectivity: # of subpool negative or bigger than number of divisions for postneurons minus 1");
    size_gp_prenrn_ = size_gp_prenrn;
    size_gp_postnrn_ = size_gp_postnrn;
}

/////////////////////////////////////////////////
/*! check if n_prenrn is in [ num_subgp_pre_/nb_division_pre_ , (num_subgp_pre_+1)/nb_division_pre_ )
 * same for n_postnrn
 * and test with proba_connection
 */
bool RandomByBlockConnectivity::do_connect(const Size & n_prenrn, const Size & n_postnrn)
{
    if ((n_prenrn*nb_division_pre_ >= num_subgp_pre_*size_gp_prenrn_) && (n_prenrn*nb_division_pre_ < (num_subgp_pre_ + 1)*size_gp_prenrn_)
            && (n_postnrn*nb_division_post_ >= num_subgp_post_*size_gp_postnrn_) && (n_postnrn*nb_division_post_ < (num_subgp_post_ + 1)*size_gp_postnrn_))
        return (RandomGenerator::dran(1.) < proba_connection_);
    else
        return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// RandomConnectivity function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//! RandomConnectivity constructor with script file
RandomConnectivity::RandomConnectivity(std::ifstream & is)
{
    std::string test;
    READ_FROM_FILE(is, proba_connection_, "proba_connection", "RandomConnectivity")
    if ((proba_connection_ < 0.) || (proba_connection_ > 1.)) throw ConfigError("RandomConnectivity: proba of connection out of [0,1]");
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// ListConnectivity function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//! constructor with script file
/*! the connection are in the script file "( n1 n2 ) ( n1 n2 ) ..."
 */
ListConnectivity::ListConnectivity(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn)
{
    con_list_ = std::vector<std::vector<bool> >(size_gp_prenrn, std::vector<bool>(size_gp_postnrn, false));
    std::string test;
    Size n1, n2;
    bool fail = false;
    while (! fail) {
        if (is.eof()) {
            fail = true; break;
        } else {
            is >> test;
            if (test == "(") {
                if (is.eof()) {
                    fail = true; break;
                } else is >> n1; // index of preneuron
                if (is.eof()) {
                    fail = true; break;
                } else is >> n2; // index of postneuron
                if (is.eof()) {
                    fail = true; break;
                } else is >> test;
                if (test == ")") {
                    if ((n1 < con_list_.size()) && (n2 < con_list_[n1].size()))
                        con_list_[n1][n2] = true;
                    else
                        throw ConfigError("ListConnectivity: at least one of the indexes is out of bound");
                } else throw ConfigError("ListConnectivity: wrong sequence in the script file, expected ')'");
            } else if (test == "ENDLIST")
                break;
        }
    }
    if (fail)
        throw ConfigError("ListConnectivity: wrong end of the script file");
}

/////////////////////////////////////////////////
//! check if connection target is in the list
bool ListConnectivity::do_connect(const Size & n_prenrn, const Size & n_postnrn)
{
    if ((n_prenrn < con_list_.size()) && (n_postnrn < con_list_[n_prenrn].size()))
        return (con_list_[n_prenrn][n_postnrn]);
    else
        throw ConfigError("ListConnectivity: unexpected discripency between the max indexes of the connection list and the groups to connect");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// LoopConnectivity function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//! constructor with script file
LoopConnectivity::LoopConnectivity(std::ifstream & is, const Size & size_gp)
{
    con_list_ = std::vector<std::vector<bool> >(size_gp, std::vector<bool>(size_gp, false));
    std::string test;
    READ_FROM_FILE(is, number_of_loops_, "number_of_loops", "LoopConnectivity")
    READ_FROM_FILE(is, length_of_loops_, "length_of_loops", "LoopConnectivity")
    if (number_of_loops_ < 0) throw ConfigError("LoopConnectivity: the number of loops must be positive");
    if (length_of_loops_ < 2) throw ConfigError("LoopConnectivity: the length of loops must be > 2");
    // creation of the loops
    for (Size i = 0; i < number_of_loops_; ++i) {
        Size * nrn_array = new Size[length_of_loops_];
        nrn_array[0] = RandomGenerator::iran(size_gp);
        for (Size j = 1; j < length_of_loops_; ++j) {
            bool test = false;
            while (! test) {
                nrn_array[j] = RandomGenerator::iran(size_gp);
                test = true;
                for (Size k = 0; k < j; ++k) {
                    if (nrn_array[j] == nrn_array[k]) {
                        test = false;
                        break;
                    }
                }
            }
            con_list_[nrn_array[j-1]][nrn_array[j]] = true;
        }
        con_list_[nrn_array[length_of_loops_-1]][nrn_array[0]] = true;
        delete nrn_array;

    }
}

/////////////////////////////////////////////////
//! check if connection in the list
bool LoopConnectivity::do_connect(const Size & n_prenrn, const Size & n_postnrn)
{
    if ((n_prenrn < con_list_.size()) && (n_postnrn < con_list_[n_prenrn].size()))
        return (con_list_[n_prenrn][n_postnrn]);
    else
        throw ConfigError("ListConnectivity: unexpected discripency between the max indexes of the connection list and the groups to connect");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// SparseRandomConnectivityTypeII function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
//! SparseRandomConnectivityTypeII constructor with script file
SparseRandomConnectivityTypeII::SparseRandomConnectivityTypeII(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn)
{

    std::string test;
    Size n_prenrn, n_postnrn;
    READ_FROM_FILE(is, num_connections_, "num_connections", "SparseRandomConnectivityTypeII")

    con_list_ = std::vector<std::vector<bool> >(size_gp_prenrn, std::vector<bool>(size_gp_postnrn, false));

    for (n_postnrn = 0; n_postnrn < size_gp_postnrn; n_postnrn++)
        for (int ic = 0; ic < num_connections_; ic++) {
            do
                n_prenrn = RandomGenerator::iran((int)size_gp_prenrn);
            while ((con_list_[n_prenrn][n_postnrn] == true) || (n_prenrn == n_postnrn));
            con_list_[n_prenrn][n_postnrn] = true;
        }
}

/////////////////////////////////////////////////
//! check if connection in the list
bool SparseRandomConnectivityTypeII::do_connect(const Size & n_prenrn, const Size & n_postnrn)
{
    if ((n_prenrn < con_list_.size()) && (n_postnrn < con_list_[n_prenrn].size()))
        return (con_list_[n_prenrn][n_postnrn]);
    else
        throw ConfigError("SparseRandomConnectivityTypeII: unexpected discripency between the max indexes of the connection list and the groups to connect");
}
