/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-15					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#ifndef Fr_CONCBUILDER_CC_INCLUDED
#define Fr_CONCBUILDER_CC_INCLUDED

#include "framepac/concbuilder.h"
#include "template/bufbuilder.cc"

namespace Fr
{

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
bool ConcurrentBufferBuilder<T,minsize>::read(const char*& input)
{
   std::lock_guard<std::mutex> guard(m_mutex) ;
   return super::read(input) ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void ConcurrentBufferBuilder<T,minsize>::append(T value)
{
   std::lock_guard<std::mutex> guard(m_mutex) ;
   this->super::append(value) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void ConcurrentBufferBuilder<T,minsize>::append(const BufferBuilder<T,minsize>& buf)
{
   std::lock_guard<std::mutex> guard(m_mutex) ;
   this->super::append(buf) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void ConcurrentBufferBuilder<T,minsize>::remove()
{
   std::lock_guard<std::mutex> guard(m_mutex) ;
   this->super::remove() ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !Fr_CONCBUILDER_CC_INCLUDED */

// end of file template/concbuilder.cc //
