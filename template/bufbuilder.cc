/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017 Carnegie Mellon University			*/
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

#include "framepac/builder.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename T, size_t minsize>
BufferBuilder<T,minsize>::~BufferBuilder()
{
   if (m_buffer != m_localbuf)
      delete [] m_buffer ;
   m_currsize = 0 ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::append(T value)
{
   if (m_currsize >= m_alloc)
      {
         size_t newalloc { 2 * m_currsize };
	 T *newbuf { new T[newalloc] };
	 if (newbuf)
	    {
	    for (size_t i = 0 ; i < m_currsize ; i++)
	       {
	       newbuf[i] = m_buffer[i] ;
	       }
	    m_buffer = newbuf ;
	    m_alloc = newalloc ;
	    }
      }
   m_buffer[m_currsize++] = value ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
T *BufferBuilder<T,minsize>::finalize() const
{
   T *finalbuf { new T[m_currsize] };
   for (size_t i = 0 ; i < m_currsize ; i++)
      {
      finalbuf[i] = m_buffer[i] ;
      }
   return finalbuf ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file template/bufbuilder.cc //
