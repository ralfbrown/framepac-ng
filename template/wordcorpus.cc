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

#include "framepac/wordcorpus.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename ID, typename Index>
WordCorpusT<ID,Index>::WordCorpusT(const char *filename, bool readonly)
   : m_wordmap(),
     m_wordbuf(),
     m_fwdindex(),
     m_revindex(),
     m_attributes(nullptr),
     m_numwords(0)
{
   (void)filename; (void)readonly ;

   return ;
}

//----------------------------------------------------------------------------

template <typename ID, typename Index>
WordCorpusT<ID,Index>::WordCorpusT(CFile &fp, bool readonly)
   : m_wordmap(),
     m_wordbuf(),
     m_fwdindex(),
     m_revindex(),
     m_attributes(nullptr),
     m_numwords(0)
{
   (void)fp; (void)readonly ;

   return ;
}

//----------------------------------------------------------------------------

template <typename ID, typename Index>
bool WordCorpusT<ID,Index>::load(CFile &fp)
{
   (void)fp ;

   return false ;//FIXME
}

//----------------------------------------------------------------------------

template <typename ID, typename Index>
bool WordCorpusT<ID,Index>::load(const char *filename)
{
   CInputFile fp(filename) ;
   return fp ? load(fp) : false ;
}

//----------------------------------------------------------------------------

template <typename ID, typename Index>
bool WordCorpusT<ID,Index>::save(CFile &fp) const
{
   (void)fp ;

   return false ;//FIXME
}

//----------------------------------------------------------------------------

template <typename ID, typename Index>
bool WordCorpusT<ID,Index>::save(const char *filename) const
{
   COutputFile fp(filename) ;
   return fp ? save(fp) : false ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file wordcorpus.C //
