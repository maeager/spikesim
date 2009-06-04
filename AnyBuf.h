#include <deque>
#include <boost/any.hpp>

using boost::any_cast;
typedef std::deque<boost::any> many;



many bbsbuf;
int ifarg(int i) { return bbsbuf.size() <= i; }
int* getint(int i){ return  boost::any_cast<int>(&bbsbuf[i]); }
double* getarg(int i){ return  (double*)boost::any_cast<double>(&bbsbuf[i]); }
double* getval(int i) { return  (double*)boost::any_cast<double>(&bbsbuf[i]); }
std::vector<double> * getvec(int i) { return boost::any_cast<std::vector<double> >(&bbsbuf[i]);}
std::string * getstr(int i){ return boost::any_cast<std::string>(&bbsbuf[i]); }


void append_int(many & values, int value)
{
    boost::any to_append = value;
    values.push_back(to_append);
}
void append_double(many & values, double value)
{
    boost::any to_append = value;
    values.push_back(to_append);
}

void append_string(many & values, const std::string & value)
{
    values.push_back(value);
}

void append_char_ptr(many & values, const char * value)
{
    values.push_back(value);
}

void append_any(many & values, const boost::any & value)
{
    values.push_back(value);
}

void append_nothing(many & values)
{
    values.push_back(boost::any());
}

bool is_empty(const boost::any & operand)
{
    return operand.empty();
}

bool is_int(const boost::any & operand)
{
    return operand.type() == typeid(int);
}

bool is_char_ptr(const boost::any & operand)
{
    try
    {
        any_cast<const char *>(operand);
        return true;
    }
    catch(const boost::bad_any_cast &)
    {
        return false;
    }
}

bool is_string(const boost::any & operand)
{
    return any_cast<std::string>(&operand);
}
/*
void count_all(many & values, std::ostream & out)
{
    out << "#empty == "
        << std::count_if(values.begin(), values.end(), is_empty) << std::endl;
    out << "#int == "
        << std::count_if(values.begin(), values.end(), is_int) << std::endl;
    out << "#const char * == "
        << std::count_if(values.begin(), values.end(), is_char_ptr) << std::endl;
    out << "#string == "
        << std::count_if(values.begin(), values.end(), is_string) << std::endl;
}
struct property
{
    property();
    property(const std::string &, const boost::any &);

    std::string name;
    boost::any value;
};

typedef std::list<property> properties;
*/