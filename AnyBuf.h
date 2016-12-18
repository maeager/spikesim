#ifndef ANYBUF_H
#define  ANYBUF_H

#include <deque>
#include <string>
#include <boost/any.hpp>

//See documentation in boost/doc/html/any/s02.html

using boost::any_cast;
typedef std::deque<boost::any> many;



static many bbsbuf;
//int ifarg(int i) { return (int)bbsbuf.size() <= i; }
//int* getint(int i){ return  boost::any_cast<int>(&bbsbuf[i]); }
// double* getarg(int i){ return  (double*)boost::any_cast<double>(&bbsbuf[i]); }
// double* getval(int i) { return  (double*)boost::any_cast<double>(&bbsbuf[i]); }
// std::vector<double> * getvec(int i) { return boost::any_cast<std::vector<double> >(&bbsbuf[i]);}
// std::string * getstr(int i){ return boost::any_cast<std::string>(&bbsbuf[i]); }

#define  ifarg(i) ((int)bbsbuf.size() <= i)
#define  getint(i) boost::any_cast<int>(&bbsbuf[i])
#define getarg(i) (double*)boost::any_cast<double>(&bbsbuf[i])
#define  getval(i)  (double*)boost::any_cast<double>(&bbsbuf[i])
#define getvec(i)  boost::any_cast<std::vector<double> >(&bbsbuf[i])
#define  getstr(i) boost::any_cast<std::string>(&bbsbuf[i])

// #define append_int(value); \
// {\
//     boost::any to_append = value;\
//     bbsbuf.push_back(to_append);\
// }
// #define append_double(value); \
// {\
//     boost::any to_append = value;\
//     bbsbuf.push_back(to_append);\
// }
//
// #define append_string(value); \
// {\
//    boost::any to_append = value;\
//     bbsbuf.push_back(to_append);\
// }
//
// #define append_char_ptr(value); \
// {\
//    boost::any to_append = value;\
//     bbsbuf.push_back(to_append);\
// }




static void append_int(int value)
{
    boost::any to_append = value;
    bbsbuf.push_back(to_append);
}
static void append_double(double value)
{
    boost::any to_append = value;
    bbsbuf.push_back(to_append);
}

static void append_string(std::string value)
{
    boost::any to_append = value;
    bbsbuf.push_back(to_append);
}

static void append_char_ptr(const char * value)
{
    boost::any to_append = value;
    bbsbuf.push_back(to_append);
}
/*
static void print_bbsbuf()
{
  int j,i=1;
    boost::any elem;
    std::cout <<"[print bbsbuf ]"<< std::endl;
 while (ifarg(i)){
   if(std::string *s = getstr(i)){
     std::cout<< i <<": "<<*s<<std::endl;
   }else if(int *s = getint(i)){
     std::cout<< i <<": "<<(int)*s<<std::endl;
   }else if(double *s = getval(i)){
     std::cout<< i <<": "<<(double)*s<<std::endl;
   }else if(std::vector *s = getvec(i)){
     for (j=0; j< s->size(); j++)
     std::cout<< i <<": "<<*s[j]<<std::endl;
   }else
     {
       std::cout<<"type mismatch"<<endl;
     }
   
   i++;
 }
    std::cout <<"[bbsbuf end] "<< std::endl;
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

#endif
