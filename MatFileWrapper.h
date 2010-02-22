#ifndef MATFILEWRAPPER_H
#define MATFILEWRAPPER_H

#ifdef MAT_FILE_OUTPUT

#include <string>
#include <vector>

#include <mat.h>

//! Describes how the output data should be translated into a MATLAB type
enum MatFileFormat {
    //! Output should be a 2D matrix of real numbers
    /*! Used for outputting rate, weight or potential.

        The rows of the cell array are a time series, columns represent
        individual neurons or synapses.
    */
    MATFILE_MATRIX,
    //! Output should be a 2D cell-array of column vectors
    /*! Used for outputting spike times.

        The rows of the cell array are a time series, columns represent
        individual neurons.

        Each cell contains a column vector of spike times for a particular
        neuron during a particular time interval.
    */
    MATFILE_SPIKE
};

//! MatFileInitialiser: encapsulates parameters of MatFileWrapper.
class MatFileInitialiser
{
    friend class MatFileWrapper;
public:
    //! Constructor.
    /*! \param[in] name Name of this output variable. Filename will be \a name + ".mat".
        \param[in] format Specifies how the output should be translated into Matlab types
    */
    MatFileInitialiser(const std::string & name, const MatFileFormat & format)
            : name_(name)
            , format_(format) {}
    //! Copy constructor.
    MatFileInitialiser(const MatFileInitialiser & s)
            : name_(s.name_)
            , format_(s.format_) {}
private:
    std::string file_name(void) const {
        return std::string(name_ + ".mat");
    }
    const std::string name_; /*!< File name. */
    const MatFileFormat format_;
};

//! MatFileWrapper: manages the access to a MAT-file.
/*! This class has a protected constructor, and shall only be called by OutputterImpl.
 */
class MatFileWrapper
{
protected:
    //! Type of initialisation value.
    typedef MatFileInitialiser InitValue;

    //! Constructor.
    MatFileWrapper(InitValue init_param) : init_param_(init_param) {}

    //! Initialise output
    inline void open_file() {
        rows = col = 0;
    }

    //! Finalise output
    inline void close_file() {
        mxArray *matrix;
        if (init_param_.format_==MATFILE_MATRIX) {
            // Create a 2D matrix of doubles
            matrix = mxCreateDoubleMatrix(rows, cols, mxREAL);
            // Arrange all the data from 'data' in it, in column order
            double *pr = mxGetPr(matrix);
            for (int r=0;r<rows;r++) {
                for (int c=0;c<cols;c++) {
                    pr[c*rows+r] = data[r*cols+c]; 
                }
            }
        } else {
            // Create a 2D matrix of cells
            matrix = mxCreateCellMatrix(rows,cols);
            // Arrange all the vectors from 'spikeData' in it, in column order
            for (int r=0;r<rows;r++) {
                for (int c=0;c<cols;c++) {
                    mxSetCell(matrix,c*rows+r,spikeData[r*cols+c]);
                }
            }
        }
        // Write matrix to file
        MATFile *file = matOpen(init_param_.file_name().c_str(),"w");
        matPutVariable(file,init_param_.name_.c_str(),matrix);
        matClose(file);
        // Free the matrix (and anything in it)
        mxDestroyArray(matrix);
    }

public:
    //! Empty method, not used here.
    /*! Used by the neuron handler to specify the number of columns of output.
     */
    void info_from_neuron_handler(const unsigned &) {}

    //! Append a double to the output
    inline void write_to_file(const double & val) {
        data.push_back(val);
        if (init_param_.format_==MATFILE_MATRIX) {
            col++;
        }
    }

    //! Mark the end of a section of output
    /*! insert_separation("end_output") marks the end of a row (end of a time step).

        When using MatFileFormat::MATFILE_SPIKE, insert_separation("end_variable") marks the end of a vector of spike times.
     */
    inline void insert_separation(const std::string & mode) {
        if (mode=="end_variable") {
            mxArray *vector = mxCreateDoubleMatrix(data.size(), 1, mxREAL);
            double *pr = mxGetPr(vector);
            for (int i=0;i<data.size();i++) {
                pr[i] = data[i];
            }
            data.clear();
            spikeData.push_back(vector);
            col++;
        } else {
            if (rows==0) {
                cols = col;
            } else {
                mxAssert(cols==col,"MatFileWrapper: matrix not rectangular");
            }
            col = 0;
            rows++;
        }
    }

private:
    //! File initialisation object.
    const MatFileInitialiser init_param_;
    int cols, rows, col;
    std::vector<double> data;
    std::vector<mxArray *> spikeData;
};

#endif  // defined(MAT_FILE_OUTPUT)

#endif  // !defined(MATFILEWRAPPER_H)
