/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-20					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
/*	This program may be redistributed and/or modified under the	*/
/*	terms of the GNU General Public License, version 3, or an	*/
/*	alternative license agreement as detailed in the accompanying	*/
/*	file LICENSE.  You should also have received a copy of the	*/
/*	GPL (file COPYING) along with this program.  If not, see	*/
/*	http://www.gnu.org/licenses/					*/
/*									*/
/*	This program is distributed in the hope that it will be		*/
/*	useful, but WITHOUT ANY WARRANTY; without even the implied	*/
/*	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR		*/
/*	PURPOSE.  See the GNU General Public License for more details.	*/
/*									*/
/************************************************************************/

#ifndef _Fr_MATRIX_H_INCLUDED
#define _Fr_MATRIX_H_INCLUDED

#include "framepac/object.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename T>
class Matrix : public Object
   {
   public: // types
      typedef Object super ;

   public:
      // object factories
      static Matrix* create(size_t rows, size_t cols) ;
      virtual T& operator() (size_t row, size_t col) ;
      virtual const T& operator() (size_t row, size_t col) const ;

   protected:
      // *** creation/destruction ***
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      Matrix(size_t rows, size_t cols) : m_rows(rows), m_cols(cols) {}
      Matrix(const Matrix&) = default ;
      virtual ~Matrix() = default ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Matrix> ;

      // *** type determination predicates ***
      static bool isMatrix_(const Object*) { return true ; }

      // *** copying ***

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<Matrix*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { free_(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer

      // *** standard info functions ***
      static size_t size_(const Object* obj)
	 {
	    const Matrix* m = static_cast<const Matrix*>(obj) ;
	    return m->m_rows * m->m_cols ;
	 }
      static size_t empty_(const Object* obj) { return Matrix::size_(obj) == 0 ; }

      // *** standard access functions ***
      static void matrixSet_(Object* o, size_t row, size_t col, double value)
	 { static_cast<Matrix*>(o)(row,col) = T(value) ; }
      static double matrixGet_(const Object* o, size_t row, size_t col)
	 { return static_cast<Matrix*>(o)(row,col) ; }

      // *** comparison functions ***

      // *** iterator support ***

      // *** STL compatibiity ***

   protected: // data members
      size_t m_rows ;
      size_t m_cols ;
   private: // static data
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

template <typename T>
Allocator Matrix<T>::s_allocator(FramepaC::Object_VMT<Matrix<T>>::instance(),sizeof(Matrix<T>)) ;
template <typename T>
const char Matrix<T>::s_typename[] = "Matrix" ;

/************************************************************************/
/************************************************************************/

template <typename T>
class FullMatrix : public Matrix<T>
   {
   public: // types
      typedef Matrix<T> super ;

   public:
      // object factories
      static FullMatrix* create(size_t rows, size_t cols) ;
      virtual T& operator() (size_t row, size_t col) { return m_matrix[row*this->m_cols + col] ; }
      virtual const T& operator() (size_t row, size_t col) const { return m_matrix[row*this->m_cols + col] ; }

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      FullMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      FullMatrix(const FullMatrix&) ;
      virtual ~FullMatrix() ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<FullMatrix> ;

      // *** copying ***

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<FullMatrix*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { free_(obj) ; }

   protected: // data members
      T* m_matrix ;
   private: // static data
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

template <typename T>
Allocator FullMatrix<T>::s_allocator(FramepaC::Object_VMT<FullMatrix<T>>::instance(),sizeof(FullMatrix<T>)) ;
template <typename T>
const char FullMatrix<T>::s_typename[] = "FullMatrix" ;

/************************************************************************/
/************************************************************************/

template <typename T>
class SparseMatrix : public Matrix<T>
   {
   public: // types
      typedef Matrix<T> super ;

   public:
      // object factories
      static SparseMatrix* create(size_t rows, size_t cols) ;
      
   protected:
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      SparseMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      SparseMatrix(const SparseMatrix&) ;
      virtual ~SparseMatrix() ;

   private: // static data
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

template <typename T>
const char SparseMatrix<T>::s_typename[] = "SparseMatrix" ;

/************************************************************************/
/************************************************************************/

template <typename T>
class UpperTriangMatrix : public Matrix<T>
   {
   public: // types
      typedef Matrix<T> super ;

   public:
      // object factories
      static UpperTriangMatrix* create(size_t rows, size_t cols) ;

      virtual T& operator() (size_t row, size_t col)
	 { if (col < row) return m_dummy ;
           return m_matrix_rows[row][col-row] ; }
      virtual const T& operator() (size_t row, size_t col) const
	 { if (col < row) return m_dummy ;
           return m_matrix_rows[row][col-row] ; }

   protected:

   protected:
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      UpperTriangMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      UpperTriangMatrix(const UpperTriangMatrix&) ;
      virtual ~UpperTriangMatrix() ;

   protected: // data members
      T  m_dummy ;
      T** m_matrix_rows ;
   private: // static data
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

template <typename T>
const char UpperTriangMatrix<T>::s_typename[] = "UpperTriangMatrix" ;

/************************************************************************/
/************************************************************************/

template <typename T>
class LowerTriangMatrix : public Matrix<T>
   {
   // it's tempting to just swap the coordinates on UpperTriangMatrix, but that
   //   would lead to poor cache behavior in many cases
   public: // types
      typedef Matrix<T> super ;

   public:
      // object factories
      static LowerTriangMatrix* create(size_t rows, size_t cols) ;
      
   protected:
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      LowerTriangMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      LowerTriangMatrix(const LowerTriangMatrix&) ;
      ~LowerTriangMatrix() ;

   protected: // data members
      T** m_matrix_cols ;
   private: // static data
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

template <typename T>
const char LowerTriangMatrix<T>::s_typename[] = "LowerTriangMatrix" ;

/************************************************************************/
/************************************************************************/

template <typename T>
class SymmetricMatrix : public UpperTriangMatrix<T>
   {
   public: // types
      typedef UpperTriangMatrix<T> super ;

   public:
      // object factories
      static SymmetricMatrix* create(size_t rows, size_t cols) ;
      
      virtual T& operator() (size_t row, size_t col)
	 { if (col < row) std::swap(row,col) ;
	   return this->m_matrix_rows[row][col-row] ; }
      virtual const T& operator() (size_t row, size_t col) const
	 { if (col < row) std::swap(row,col) ;
	   return this->m_matrix_rows[row][col-row] ; }

   protected:
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      SymmetricMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      SymmetricMatrix(const SymmetricMatrix&) ;
      virtual ~SymmetricMatrix() = default ;

   private: // static data
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

template <typename T>
const char SymmetricMatrix<T>::s_typename[] = "SymmetricMatrix" ;

/************************************************************************/
/************************************************************************/

template <typename T>
class DiagonalMatrix : public Matrix<T>
   {
   public: // types
      typedef Matrix<T> super ;

   public:
      // object factories
      static DiagonalMatrix* create(size_t rows, size_t cols) ;
      
   protected:
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      DiagonalMatrix(size_t rows, size_t cols, size_t width) : super(rows,cols) {} //FIXME
      DiagonalMatrix(const DiagonalMatrix&) ;
      virtual ~DiagonalMatrix() ;

      virtual T& operator() (size_t row, size_t col)
	 { if (col < row) std::swap(row,col) ;
	   return this->m_matrix_rows[row][col-row] ; }

   protected:
      size_t m_width ;   // width of the band around the diagonal which is non-zero
			 // m_width=0 is just the diagonal, m_width=1 is diag plus one on each side, etc.
      T* m_matrix_elts ;
      T  m_dummy ;
   private: // static data
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

template <typename T>
const char DiagonalMatrix<T>::s_typename[] = "DiagonalMatrix" ;

//----------------------------------------------------------------------------

} ; //end namespace Fr

#endif /* !_Fr_MATRIX_H_INCLUDED */

// end of file matrix.h //
