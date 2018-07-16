/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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
      
   protected:
      Matrix(size_t rows, size_t cols) : m_rows(rows), m_cols(cols) {}
      Matrix(const Matrix &) = default ;
      ~Matrix() = default ;

   protected:
      size_t m_rows ;
      size_t m_cols ;
   } ;

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
      
   protected:
      FullMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      ~FullMatrix() ;

   } ;

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
      SparseMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      ~SparseMatrix() ;

   } ;

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
      
   protected:

   public:
      UpperTriangMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      ~UpperTriangMatrix() ;

   } ;

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
      LowerTriangMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      ~LowerTriangMatrix() ;

   } ;

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
      
   protected:
      SymmetricMatrix(size_t rows, size_t cols) : super(rows,cols) {} //FIXME
      ~SymmetricMatrix() ;

   } ;

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
      DiagonalMatrix(size_t rows, size_t cols, size_t width) : super(rows,cols) {} //FIXME
      ~DiagonalMatrix() ;

   protected:
      size_t m_width ;   // width of the band around the diagonal which is non-zero
			 // m_width=0 is just the diagonal, m_width=1 is diag plus one on each side, etc.
   } ;

//----------------------------------------------------------------------------

} ; //end namespace Fr

#endif /* !_Fr_MATRIX_H_INCLUDED */

// end of file matrix.h //
