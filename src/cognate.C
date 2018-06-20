/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-06-19					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018 Carnegie Mellon University			*/
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
#include "framepac/spelling.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class CognateData					*/
/************************************************************************/

CognateData::CognateData()
{
   for (size_t row = 0; row < 256 ; ++row)
      {
      // by default, no benefit from any insertions/deletions/substitutions
      m_insertion[row] = 0.0 ;
      m_deletion[row] = 0.0 ;
      for (size_t col = 0; col < 256 ; ++col)
	 {
	 m_one2one[row][col] = 0.0 ;
	 }
      // but "substituting" a byte for itself gets full weight
      m_one2one[row][row] = 1.0 ;
      }
   return ;
}

//----------------------------------------------------------------------------

CognateData::CognateData(const char* cognates, size_t fuzzy_match_score)
   : CognateData()
{
   (void)fuzzy_match_score; //FIXME
   setCognateScoring(cognates) ;

   return ;
}

//----------------------------------------------------------------------------

CognateData::CognateData(const List* cognates, size_t fuzzy_match_score)
   : CognateData()
{
   (void)fuzzy_match_score; //FIXME
   setCognateScoring(cognates) ;

   return ;
}

//----------------------------------------------------------------------------

CognateData::~CognateData()
{

   return ;
}

//----------------------------------------------------------------------------

CognateData* CognateData::defaultInstance()
{
   static CognateData instance ;

   return &instance ;
}

//----------------------------------------------------------------------------

CognateData* CognateData::load(const char* filename, size_t fuzzy_match_score)
{
   CInputFile f(filename) ;
   List* cognates = nullptr ;
   if (f)
      {
      cognates = nullptr ; //FIXME
      }
   CognateData* cog = nullptr ;
   if (!filename || !*filename || f)
      {
      cog = new CognateData(cognates,fuzzy_match_score) ;
      }
   return cog ;
}

//----------------------------------------------------------------------------

bool CognateData::save(const char* /*filename*/)
{

   return false ; //FIXME
}

//----------------------------------------------------------------------------

bool CognateData::setCognateScoring(const char* /*cognates*/)
{

   return false ; //FIXME
}

//----------------------------------------------------------------------------

bool CognateData::setOne2ManyScore(char /*source*/, const char* /*targets*/)
{

   return false ; //FIXME
}

//----------------------------------------------------------------------------

bool CognateData::setMany2OneScore(const char* /*sources*/, char /*target*/)
{

   return false ; //FIXME
}

//----------------------------------------------------------------------------

bool CognateData::areCognate(char letter1, char letter2, bool case_fold) const
{
   if (case_fold)
      {
      letter1 = tolower(letter1) ;
      letter2 = tolower(letter2) ;
      }
   return m_one2one[(unsigned)letter1][(unsigned)letter2] > 0 ;
}

//----------------------------------------------------------------------------

size_t CognateData::cognateLetters(const char* str1, const char* str2, size_t& len1, size_t& len2) const
{
   (void)str1; (void)str2; (void)len1; (void)len2;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

static double** alloc_score_buffer(size_t rows, size_t columns)
{
   double** buffer = new double*[rows] ;
   if (!buffer)
      return buffer ;
   for (size_t i = 0 ; i < rows ; ++i)
      buffer[i] = new double[columns] ;
   return buffer ;
}

//----------------------------------------------------------------------------

static void free_score_buffer(double** buffer, size_t rows)
{
   if (!buffer)
      return;
   for (size_t i = 0 ; i < rows ; ++i)
      delete[] buffer[i] ;
   delete buffer ;
   return ;
}

//----------------------------------------------------------------------------

static double score_exact_matches(const char* word1, size_t len1, const char* word2, size_t len2,
   double** score_buf, CognateAlignment** /*align*/)
{
   // initialize the first row of the scoring matrix
   for (size_t col = 0 ; col < len2 ; ++col)
      {
      score_buf[0][col] = 0.0 ;
      }
   // now iterate each row, checking for best choice of substitute/delete/insert
   for (size_t row = 1 ; row < len1 ; ++row)
      {
      score_buf[row%2][0] = score_buf[(row-1)%2][0] ;
      for (size_t col = 1 ; col < len2 ; ++col)
	 {
	 double ins = score_buf[row%2][col-1] ;
	 double del = score_buf[(row-1)%2][col] ;
	 double subst = score_buf[(row-1)%2][col-1] + (word1[row] == word2[col]) ? 1.0 : 0.0 ;
	 score_buf[row%2][col] = std::max(std::max(ins,del),subst) ;
	 }
      }
   return score_buf[(len1-1)%2][len2-1] ;
}

//----------------------------------------------------------------------------

double CognateData::score_single_byte(const char* word1, size_t len1, const char* word2, size_t len2,
   double** score_buf, CognateAlignment** /*align*/) const
{
   // initialize the first row of the scoring matrix
   score_buf[0][0] = 0.0 ;
   for (size_t col = 0 ; col < len2 ; ++col)
      {
      score_buf[0][col] = score_buf[0][col-1] + m_insertion[(unsigned char)word2[col-1]] ;
      }
   // now iterate each row, checking for best choice of substitute/delete/insert
   for (size_t row = 1 ; row < len1 ; ++row)
      {
      score_buf[row%2][0] = score_buf[(row-1)%2][0] + m_deletion[(unsigned char)word1[row-1]] ;
      for (size_t col = 1 ; col < len2 ; ++col)
	 {
	 double ins = score_buf[row%2][col-1] + m_insertion[(unsigned char)word2[col-1]] ;
	 double del = score_buf[(row-1)%2][col] + m_deletion[(unsigned char)word1[row-1]] ;
	 double subst = score_buf[(row-1)%2][col-1]
	    + m_one2one[(unsigned char)word1[row]][(unsigned char)word2[col]] ;
	 score_buf[row%2][col] = std::max(std::max(ins,del),subst) ;
	 }
      }
   return score_buf[(len1-1)%2][len2-1] ;
}

//----------------------------------------------------------------------------

double CognateData::score_general(const char* word1, size_t len1, const char* word2, size_t len2,
   double** score_buf, CognateAlignment** /*align*/) const
{
   size_t rows = longestSource() + 1 ;
   // initialize the first row of the scoring matrix
   score_buf[0][0] = 0.0 ;
   for (size_t col = 1 ; col < len2 ; ++col)
      {
      score_buf[0][col] = score_buf[0][col-1] + m_insertion[(unsigned char)word2[col-1]] ;
      }
   // now iterate each row, checking for best choice of substitute/delete/insert
   for (size_t row = 1 ; row < len1 ; ++row)
      {
      score_buf[row%rows][0] = score_buf[(row-1)%rows][0] + m_deletion[(unsigned char)word1[row-1]] ;
      for (size_t col = 1 ; col < len2 ; ++col)
	 {
	 double ins = score_buf[row%2][col-1] + m_insertion[(unsigned char)word2[col-1]] ;
	 double del = score_buf[(row-1)%2][col] + m_deletion[(unsigned char)word1[row-1]] ;
	 double subst = score_buf[(row-1)%2][col-1]
	    + m_one2one[(unsigned char)word1[row]][(unsigned char)word2[col]] ;
	 double subst2 = 0.0 ; //TODO
	 score_buf[row%2][col] = std::max(std::max(ins,del),std::max(subst,subst2)) ;
	 }
      }
   return score_buf[(len1-1)%rows][len2-1] ;
}

//----------------------------------------------------------------------------

double CognateData::scaledScore(double rawscore, const char* word1, const char* word2) const
{
   size_t len1 = word1 ? strlen(word1) : 0 ;
   size_t len2 = word2 ? strlen(word2) : 0 ;
   double len ;
   if (m_rel_to_shorter)
      {
      len = std::min(len1,len2) ;
      }
   else if (m_rel_to_average)
      {
      len = (len1 + len2) / 2.0 ;
      }
   else
      len = std::max(len1,len2) ;
   return len > 0 ? rawscore / len : 0.0 ;
}

//----------------------------------------------------------------------------

double CognateData::score(const char* word1, const char* word2, bool exact_letter_match_only,
   CognateAlignment** align) const
{
   if (!word1 || !word2 || (!*word1 && !*word2))
      return 0.0 ;
   double** score_buf = alloc_score_buffer(longestSource()+1,strlen(word2)+1) ;
   if (align)
      *align = nullptr ;  //FIXME
   double cogscore ;
   size_t len1 = strlen(word1) ;
   size_t len2 = strlen(word2) ;
   if (exact_letter_match_only)
      {
      cogscore = score_exact_matches(word1,len1,word2,len2,score_buf,align) ;
      }
   else if (!m_mappings)
      {
      cogscore = score_single_byte(word1,len1,word2,len2,score_buf,align) ;
      }
   else
      {
      cogscore = score_general(word1,len1,word2,len2,score_buf,align) ;
      }
   free_score_buffer(score_buf,longestSource()+1) ;
   return scaledScore(cogscore,word1,word2) ;
}

//----------------------------------------------------------------------------

double CognateData::score(const Object* word1, const Object* word2, bool exact_letter_match_only,
   CognateAlignment** align) const
{
   return (word1 && word2) ? score(word1->stringValue(),word2->stringValue(),exact_letter_match_only,align) : 0.0 ;
}

//----------------------------------------------------------------------------



} // end namespace Fr


// end of file cognate.C //
