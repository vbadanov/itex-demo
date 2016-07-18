
#ifndef MATRIX_H_
#define MATRIX_H_

#include <aux/slice-iter.hpp>

namespace aux
{

/*===========================================================================*/
template<typename T>
class matrix
{
    protected:
        std::valarray<T> *v;
        size_t dRow, dCol;

    public:
/*===========================================================================*/
        enum MultMethod
        {
            MULT_REGULAR  = 0,
            MULT_FAST     = 1
        };

/*===========================================================================*/
        matrix() : dRow(1), dCol(1)
        {
            v = new std::valarray<T>(1);
        };

/*===========================================================================*/
        matrix(size_t row, size_t col) : dRow(row), dCol(col)
        {
            v = NULL;
            v = new std::valarray<T>(dRow*dCol);
        };

/*===========================================================================*/
        matrix(const matrix& m)
        {
            v = NULL;
            v = new std::valarray<T>(m.size());
            *this = m;
        };

/*===========================================================================*/
        ~matrix() { delete v; };

/*===========================================================================*/
        void resize(size_t row, size_t col) { dRow = row; dCol = col; if(v != NULL) {delete v; v=NULL; }; v = new std::valarray<T>(dRow*dCol); }
        size_t size() const { return dRow*dCol; }
        size_t numRows() const { return dRow; }
        size_t numCols() const { return dCol; }

/*===========================================================================*/
        slice_iter<T> get_row(size_t i) { return slice_iter<T>(v,std::slice(i, dRow, dCol)); }
        slice_iter<T> get_column(size_t i) { return slice_iter<T>(v,std::slice(i*dCol, dCol, 1)); }

/*===========================================================================*/
        T& operator()(size_t row, size_t col) { return v->operator[](row*dCol + col); }      //C's style indexing
        T operator()(size_t row, size_t col) const { return v->operator[](row*dCol + col); }

        slice_iter<T> operator()(size_t i) { return get_row(i); }
        slice_iter<T> operator[](size_t i) { return get_row(i); }  //C's style indexing (? - need to check)

/*===========================================================================*/
        matrix<T>& operator*=(T num) { (*v)*=num; return *this; };
        matrix<T>& operator=(const matrix& m)
        {
            if((this->dRow != m.dRow) || (this->dCol != m.dCol))
            {
                this->resize(m.dRow, m.dCol);
            }
            unsigned long size = this->dRow * this->dCol;
            for(unsigned long i = 0; i < size; i++)
            {
                this->v->operator[](i) = m.v->operator[](i);
            }
            return *this;
        };

/*===========================================================================*/
        std::valarray<T>* array() { return v; }

/*===========================================================================*/
        matrix<T>& mult(matrix<T> &mG, matrix<T> &mH, MultMethod mulMethod = MULT_FAST)
        {
            /*
             * This class method performs multiplication of the matrixes:
             *
             * G of size (a x b)
             * H of size (b x c)
             *
             * And put result to:
             *
             * R of size (a x c)  - this is the current matrix (*this)
             *
            */
            unsigned long i,j,k;

            unsigned long a = mG.numRows();
            unsigned long b = mG.numCols();
            unsigned long c = mH.numCols();

            // Check whether it's possible to multiply matrixes G and H and put result to (*this) matrix
            if(b != mH.numRows()) { return *this; } /* What else to return? :-) */
            if((this->numRows() != a ) || (this->numCols() != c)) { this->resize(a, c); }

            matrix<T> &mR = *this;

            switch(mulMethod)
            {
                case MULT_REGULAR:
                    /*
                     * Using regular method
                    */
                    for(i=0; i<a; i++)
                    {
                        for(j=0; j<c; j++)
                        {
                            mR(i,j)=0;
                            for(k=0; k<b; k++)
                            {
                                mR(i,j) += mG(i,k) * mH(k,j);
                            }
                        }
                    }


                case MULT_FAST:
                default:
                    /*
                     * Using Vinograd's method
                    */
                    unsigned long d = b/2;
                    std::valarray<T> rowFactor(a);
                    std::valarray<T> columnFactor(c);

                    // Calculating rowFactors for G
                    for(i=0; i<a; i++)
                    {
                        rowFactor[i] = mG(i, 0) * mG(i, 1);
                        for(j=1; j<d; j++)
                        {
                            rowFactor[i] += mG(i, 2*j) * mG(i, 2*j+1);
                        };
                    };

                    // Calculating columnFactors for �
                    for(i=0; i<c; i++)
                    {
                        columnFactor[i] = mH(0, i) * mH(1, i);
                        for(j=1; j<d; j++)
                        {
                            columnFactor[i] += mH(2*j, i) * mH(2*j+1, i);
                        }
                    }

                    // Calculating matrix R
                    for(i=0; i<a; i++)
                    {
                        for(j=0; j<c; j++)
                        {
                            mR(i,j) = -rowFactor[i] - columnFactor[j];
                            for(k=0; k<d; k++)
                            {
                                mR(i,j) += (mG(i, 2*k) + mH(2*k+1, j)) * (mG(i, 2*k+1) + mH(2*k, j));
                            }
                        }
                    }

                    // Additional in case of odd common dimension
                    if(2*(b/2) != b)
                    {
                        for(i=0; i<a; i++)
                        {
                            for(j=0; j<c; j++)
                            {
                                mR(i,j) += mG(i, b-1) * mH(b-1, j);
                            }
                        }
                    }

            }

            return *this;
        };


/*===========================================================================*/
        T max_abs()
        {
            T max_value = 0;
            T curr_value = 0;
            for(size_t i = 0; i < dRow; i++)
            {
                for(size_t j = 0; j < dCol; j++)
                {
                    curr_value = abs(this->operator()(i,j));
                    if(curr_value > max_value)
                    {
                        max_value = curr_value;
                    };
                }
            }
            return max_value;
        };

/*===========================================================================*/
        T sum()
        {
            T sum = 0;
            for(size_t i = 0; i < dRow; i++)
            {
                for(size_t j = 0; j < dCol; j++)
                {
                    sum += this->operator()(i,j);
                };
            };
            return sum;
        };

/*===========================================================================*/
        T mean()
        {
            return this->sum() / (T)(dRow*dCol);
        }
};

}

#endif /*MATRIX_H_*/
