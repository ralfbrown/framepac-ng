/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-07-13					*/
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
      delete[] m_lines[i] ;
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
      delete[] ((char*)ln) ;
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

//----------------------------------------------------------------------------

bool LineBatch::append(CharPtr&& ln)
{
   if (size() >= capacity())
      {
      size_t newsize = capacity() * 2 ;
      if (newsize < 1000) newsize = 1000 ;
      if (!expandTo(newsize))
	 return false ;
      }
   m_lines[m_count++] = ln.move() ;
   return true ;
}

//----------------------------------------------------------------------------

bool LineBatch::applyVA(LineEditFunc* fn, va_list args)
{
   if (!fn)
      return false ;
   for (size_t i = 0 ; i < size() ; ++i)
      {
      std::va_list argcopy ;
      va_copy(argcopy,args) ;
      char* edited = fn(m_lines[i],argcopy) ;
      if (edited)
	 m_lines[i] = edited ;
      else
	 return false ;
      }
   return true ;
}

/************************************************************************/
/*	Methods for class CFile						*/
/************************************************************************/

LineBatch* CFile::getLines(size_t batchsize)
{
   LineBatch* batch = new LineBatch(batchsize) ;
   while (!eof() && batch->size() < batch->capacity())
      {
      batch->append(getCLine()) ;
      }
   return batch ;
}

//----------------------------------------------------------------------------

LineBatch* CFile::getLines(size_t batchsize, int mono_skip)
{
   LineBatch* batch = new LineBatch(batchsize) ;
   if (batch)
      {
      while (!eof() && batch->size() < batch->capacity())
	 {
	 // read the next non-blank line from the file
	 // if mono_skip < 0, we skip the first of each pair and read the second
	 // if mono_skip > 0, we read the first of each pair and skip the second
	 skipBlankLines() ;
	 skipLines((mono_skip<0) ? 1 : 0) ;
	 CharPtr line { getTrimmedLine() } ;
	 skipLines((mono_skip>0) ? 1 : 0) ;
	 if (line && **line)
	    batch->append(line) ;
	 }
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
