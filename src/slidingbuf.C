/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-29					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#include "framepac/file.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class SlidingBuffer					*/
/************************************************************************/

SlidingBuffer::SlidingBuffer(CFile& f, size_t bufsize, size_t overlap)
   : m_fp(f), m_buffer(bufsize), m_alloc(bufsize), m_overlap(overlap)
{
   if (f)
      {
      m_bufsize = f.read(m_buffer.begin(),bufsize) ;
//TODO
      }
   else
      {
      m_bufsize = 0 ;
      }
   return ;
}

//----------------------------------------------------------------------

const char* SlidingBuffer::at() const
{
   if (m_curpos < m_bufpos || m_curpos + m_overlap > m_bufpos + m_bufsize)
      return nullptr ;
   return m_buffer.at(m_curpos - m_bufpos) ;
}

//----------------------------------------------------------------------

bool SlidingBuffer::seek(off_t ofs)
{
   if (ofs < m_bufpos)
      return false ;			// no backward seeking
//TODO
   return true ;
}

//----------------------------------------------------------------------

bool SlidingBuffer::refill(size_t shift)
{
   if (shift >= m_bufsize)
      {
      // just refill
      if (!m_fp.seek(m_bufpos + shift))
	 return false ;
      m_bufpos += shift ;
      m_bufsize = m_fp.read(m_buffer.begin(),m_alloc) ;
      m_curpos = m_bufpos ;
      return m_bufsize > 0 ;
      }
   // shift down the remainder of the buffer
   size_t remain = m_bufsize - shift ;
   std::copy_n(m_buffer.at(shift),remain,m_buffer.begin()) ;
   // and refill
   size_t count = m_fp.read(m_buffer.at(remain),m_alloc-remain) ;
   m_bufsize = remain + count ;
   m_bufpos += shift ;
   m_curpos = m_bufpos ;
   return count > 0 ;
}

//----------------------------------------------------------------------

} // end namespace Fr

// end of file slidingbuf.C //
