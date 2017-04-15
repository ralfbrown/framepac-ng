/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-14					*/
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

#include "framepac/list.h"
#include "framepac/file.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class LineBatch					*/
/************************************************************************/

LineBatch::LineBatch(size_t init_capacity)
{
   if (init_capacity == 0) init_capacity = 1000 ;
   m_lines = New<char*>(init_capacity) ;
   m_count = 0 ;
   if (m_lines)
      {
      m_capacity = init_capacity ;
      }
   else
      {
      m_capacity = 0 ;
      }
   return ;
}

//----------------------------------------------------------------------------

LineBatch::~LineBatch()
{
   for (size_t i = 0 ; i < size() ; ++i)
      {
      Free(m_lines[i]) ;
      }
   m_count = 0 ;
   Free(m_lines) ;
   m_capacity = 0 ;
   m_lines = nullptr ;
   return ;
}

//----------------------------------------------------------------------------

void LineBatch::clear()
{
   for (auto ln : *this)
      {
      Free((char*)ln) ;
      }
   m_count = 0 ;
   return ;
}

//----------------------------------------------------------------------------

bool LineBatch::expandTo(size_t newsize)
{
   if (newsize <= capacity())
      return true ;
   char** new_lines = NewR<char*>(m_lines,newsize) ;
   if (!new_lines)
      {
      return false ;
      }
   m_lines = new_lines ;
   m_capacity = newsize ;
   return true ;
}

//----------------------------------------------------------------------------

bool LineBatch::append(char* ln)
{
   if (size() >= capacity())
      {
      size_t newsize = capacity() * 2 ;
      if (newsize < 1000) newsize = 1000 ;
      if (!expandTo(newsize))
	 return false ;
      }
   m_lines[m_count++] = ln ;
   return true ;
}

/************************************************************************/
/*	Methods for class CFile						*/
/************************************************************************/

LineBatch* CFile::getlines(size_t batchsize)
{
   LineBatch* batch = new LineBatch(batchsize) ;
   while (!eof() && batch->size() < batch->capacity())
      {
      char* line = getCLine() ;
      batch->append(line) ;
      }
   return batch ;
}

//----------------------------------------------------------------------------

void CFile::putlines(const LineBatch* batch)
{
   if (!batch)
      return ;
   for (const char* line : *batch)
      {
      if (!line) continue ;
      this->puts(line) ;
      this->putc('\n') ;
      }
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file linebatch.C //
