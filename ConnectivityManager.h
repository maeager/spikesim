// ConnectivityManager.h
//////////////////////////////////////////////////////////////////////////////////

#ifndef CONNECTIVITYMANAGER_H
#define CONNECTIVITYMANAGER_H

#include <fstream>
#include <vector>

#include "Error.h"
#include "GlobalDefs.h"
#include "RandomGenerator.h"




////////////////////////////////////////////////////////////////////////////////
// ConnectivityManager mother class definition
/////////////////////////////////////////////////////////////////////////////////

struct ConnectivityManager {
    virtual bool do_connect(const Size & n_prenrn, const Size & n_postnrn) = 0;
};



////////////////////////////////////////////////////////////////////////////////
// FullConnectivity class definition
////////////////////////////////////////////////////////////////////////////////

class FullConnectivity
        : public ConnectivityManager
{
public:
    FullConnectivity(std::ifstream & is) {}
    bool do_connect(const Size & n_prenrn, const Size & n_postnrn) {
        return true;
    }
};



////////////////////////////////////////////////////////////////////////////////
// RandomFullPreConnectivity class definition
// proba_connection_ chance for each neuron to have connections, if yes full connectivity with all other neurons as preneurons
////////////////////////////////////////////////////////////////////////////////

class RandomFullPreConnectivity
        : public ConnectivityManager
{
public:
    RandomFullPreConnectivity(std::ifstream & is, const Size & size_gp_postnrn);
    bool do_connect(const Size & n_prenrn, const Size & n_postnrn);
protected:
    std::vector<bool> post_neuron_list_;
};



////////////////////////////////////////////////////////////////////////////////
// RandomFixedPreConnectivity class definition
// proba_connection_ chance for each neuron to have connections, if yes full connectivity with all other neurons as preneurons
///////////////////////////////////////////////////////////////////////////////

class RandomFixedPreConnectivity
        : public ConnectivityManager
{
public:
    RandomFixedPreConnectivity(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn);
    bool do_connect(const Size & n_prenrn, const Size & n_postnrn);
protected:
    std::vector<std::vector<bool> > con_list_;
};



////////////////////////////////////////////////////////////////////////////////////
// FullByBlockConnectivity class definition
// divide preneuron pool (resp. postneuron) in nb_division_pre_ (resp. nb_division_post_) "equal" subpools
// connects the #num_subgp_pre_ subpool of preneurons with #num_subgp_post_ subpool of postneurons
////////////////////////////////////////////////////////////////////////////////////

class FullByBlockConnectivity
        : public ConnectivityManager
{
public:
    FullByBlockConnectivity(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn);
    bool do_connect(const Size & n_prenrn, const Size & n_postnrn);
protected:
    unsigned num_subgp_pre_, nb_division_pre_, num_subgp_post_, nb_division_post_, size_gp_prenrn_, size_gp_postnrn_;
};



////////////////////////////////////////////////////////////////////////////////////
// RandomByBlockConnectivity class definition
// divide preneuron pool (resp. postneuron) in nb_division_pre_ (resp. nb_division_post_) "equal" subpools
// connects the #num_subgp_pre_ subpool of preneurons with #num_subgp_post_ subpool of postneurons with
// proba given by proba_connection
////////////////////////////////////////////////////////////////////////////////////

class RandomByBlockConnectivity
        : public ConnectivityManager
{
public:
    RandomByBlockConnectivity(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn);
    bool do_connect(const Size & n_prenrn, const Size & n_postnrn);
protected:
    unsigned num_subgp_pre_, nb_division_pre_, num_subgp_post_, nb_division_post_, size_gp_prenrn_, size_gp_postnrn_;
    double proba_connection_;
};



////////////////////////////////////////////////////////////////////////////////////
// RandomConnectivity class definition
// connection with probability proba_connection_
////////////////////////////////////////////////////////////////////////////////////

class RandomConnectivity
        : public ConnectivityManager
{
public:
    RandomConnectivity(std::ifstream & is);
    bool do_connect(const Size & n_prenrn, const Size & n_postnrn) {
        return (RandomGenerator::dran(1.) < proba_connection_);
    }
protected:
    double proba_connection_;
};



////////////////////////////////////////////////////////////////////////////////////
// ListConnectivity class definition
////////////////////////////////////////////////////////////////////////////////////

class ListConnectivity
        : public ConnectivityManager
{
public:
    ListConnectivity(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn);
    bool do_connect(const Size & n_prenrn, const Size & n_postnrn);
protected:
    std::vector<std::vector<bool> > con_list_;
};



////////////////////////////////////////////////////////////////////////////////////
// LoopConnectivity class definition
////////////////////////////////////////////////////////////////////////////////////
// creates loops by connecting a random neuron to a next one and then until reaching the desired length minus 1
// then connects back to the initial one
// each step a random neuron is drawn, checking it hasn't been picked up previously

class LoopConnectivity
        : public ConnectivityManager
{
public:
    LoopConnectivity(std::ifstream & is, const Size & size_gp);
    bool do_connect(const Size & n_prenrn, const Size & n_postnrn);
protected:
    Size number_of_loops_, length_of_loops_;
    std::vector<std::vector<bool> > con_list_;
};



////////////////////////////////////////////////////////////////////////////////////
// SparseRandomConnectivityTypeII class definition
////////////////////////////////////////////////////////////////////////////////////

class SparseRandomConnectivityTypeII
        : public ConnectivityManager
{
public:
    SparseRandomConnectivityTypeII(std::ifstream & is, const Size & size_gp_prenrn, const Size & size_gp_postnrn);
    bool do_connect(const Size & n_prenrn, const Size & n_postnrn);
protected:
    std::vector<std::vector<bool> > con_list_;
    double num_connections_;
};



#endif // !defined(CONNECTIVITYMANAGER_H)
