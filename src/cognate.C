/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-07					*/
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

#include "framepac/cstring.h"
#include "framepac/file.h"
#include "framepac/spelling.h"
#include "framepac/stringbuilder.h"

namespace Fr
{

// request explicit instantiation
template class Trie<List*> ;

/************************************************************************/
/*	Local types for this module					*/
/************************************************************************/

struct CogScoreInfo
   {
   public:
      void init(float sc, size_t src, size_t trg)
	 {
	    score = sc ;
	    srclen = (uint16_t)src ;
	    trglen = (uint16_t)trg ;
	 }
      operator float() const { return score ; }

   public:
      float    score ;
      uint16_t srclen ;
      uint16_t trglen ;
   } ;

/************************************************************************/
/*	Methods for class CognateData					*/
/************************************************************************/

CognateData::CognateData()
{
   reset() ;
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
   delete m_mappings ;
   return ;
}

//----------------------------------------------------------------------------

CognateData* CognateData::defaultInstance()
{
   static CognateData instance ;

   return &instance ;
}

//----------------------------------------------------------------------------

void CognateData::reset()
{
   for (size_t row = 0; row < 256 ; ++row)
      {
      // by default, no benefit from any insertions/deletions/substitutions
      m_insertion[row] = 0.0f ;
      m_deletion[row] = 0.0f ;
      for (size_t col = 0; col < 256 ; ++col)
	 {
	 m_one2one[row][col] = 0.0f ;
	 }
      // but "substituting" a byte for itself gets full weight
      m_one2one[row][row] = 1.0f ;
      }
   return ;
}

//----------------------------------------------------------------------------

CognateData* CognateData::load(const char* filename, size_t fuzzy_match_score)
{
   CInputFile f(filename) ;
   List* cognates = nullptr ;
   if (f)
      {
      // determine the format of the data in the file
      // first, skip blank and comment lines
      while (!f.eof())
	 {
	 f.skipBlankLines() ;
	 int c = f.getc_nonws() ;	// check the first non-whitespace char
	 if (c == ';' || c == '#')	// is it a comment?
	    f.skipLines() ;		// consume the comment line
	 else
	    {
	    f.ungetc(c) ;
	    break ;			// not a comment, so this is the start of data
	    }
	 }
      // now check the first character; it should be either an open paren or a double quote
      int c = f.getc() ;
      f.ungetc(c) ;
      if (c == '(')
	 {
	 // the file contains a single FramepaC list
	 StringBuilder sb ;
	 while (!f.eof())
	    {
	    sb += *f.getCLine() ;
	    }
	 const char* cogstring = sb.c_str() ;
	 cognates = List::create(cogstring) ;
	 }
      else if (c == '"')
	 {
	 // the file contains one line per substitution, in the format
	 //   "src" "trg" score
	 while (!f.eof())
	    {
	    CharPtr line { f.getCLine() } ;
	    //TODO
	    }
	 }
      else
	 {
	 // error, wrong format
	 //TODO
	 }
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

bool CognateData::setDoublings(const char* allowable_letters, double score, bool reverse)
{
   if (!allowable_letters)
      return true ;			// trivially successful
   char src[2] ;
   char trg[3] ;
   src[1] = '\0' ;
   trg[2] = '\0' ;
   for ( ; *allowable_letters ; ++allowable_letters)
      {
      src[0] = trg[0] = trg[1] = *allowable_letters ;
      if (reverse)
	 {
	 if (!setCognateScoring(trg,src,score))
	    return false ;
	 }
      else if (!setCognateScoring(src,trg,score))
	 return false ;
      }
   return true ;
}

//----------------------------------------------------------------------------

bool CognateData::setTranspositions(const char* allowable_letters, double score)
{
   if (!allowable_letters)
      return true ;			// trivially successful
   for ( ; *allowable_letters ; ++allowable_letters)
      {
      char src[3] ;
      char trg[3] ;
      src[0] = *allowable_letters ;
      src[2] = '\0' ;
      trg[1] = *allowable_letters ;
      trg[2] = '\0' ;
      for (size_t i = 1 ; allowable_letters[i] ; ++i)
	 {
	 src[1] = allowable_letters[i] ;
	 trg[0] = allowable_letters[i] ;
	 if (!setCognateScoring(src,trg,score)
	    || !setCognateScoring(trg,src,score))
	    return false ;
	 }
      }
   return true ;
}

//----------------------------------------------------------------------------

bool CognateData::setCognateScoring(const char* src, const char* trg, double sc)
{
   size_t srclen = strlen(src) ;
   size_t trglen = strlen(trg) ;
   if (srclen == 1 && trglen == 0)
      {
      // this is a single-char deletion
      m_deletion[(unsigned)src[0]] = (float)sc ;
      return true ;
      }
   else if (srclen == 0 && trglen == 1)
      {
      // this is a single-char insertion
      m_insertion[(unsigned)trg[0]] = (float)sc ;
      return true ;
      }
   else if (srclen == 1 && trglen == 1)
      {
      // this is a single-char substitution
      m_one2one[(unsigned)src[0]][(unsigned)trg[0]] = (float)sc ;
      return true ;
      }
   else
      {
      // add to generic M-to-N substitutions
      if (!m_mappings)
	 m_mappings = new Fr::Trie<List*> ;

      return false; //FIXME
      }
}

//----------------------------------------------------------------------------

bool CognateData::setCognateScoring(const List* cognates)
{
   if (!cognates)
      return true ;			// trivially successful
   for (const Object* cog : *cognates)
      {
      // each element of 'cognates' is of the form
      //    ("src" ("trg1" sc1) ("trg2" sc2) ...)
      auto src = cog->front() ;
      if (!src) continue ;
      const char* srcstr = src->stringValue() ;
      if (!srcstr) continue ;
      auto targets = cog->next() ;
      for (auto target : *targets)
	 {
	 auto trg = target->front() ;
	 if (!trg) continue ;
	 const char* trgstr = trg->stringValue() ;
	 if (!trgstr) continue ;
	 auto sc = target->nthFloat(1) ;
	 setCognateScoring(srcstr,trgstr,sc) ;
	 }
      }
   return false; //FIXME
}

//----------------------------------------------------------------------------

bool CognateData::setCognateScoring(const char* cognates)
{
   if (!cognates || !*cognates)
      return true ;			// trivially successful
   auto cogdata = Object::create(cognates) ;
   if (cogdata == nullptr || !cogdata->isList())
      return false ;			// couldn't parse string into a list
   return setCognateScoring(reinterpret_cast<List*>(&cogdata)) ;
}

//----------------------------------------------------------------------------

bool CognateData::areCognate(char letter1, char letter2, bool case_fold) const
{
   if (case_fold)
      {
      letter1 = tolower(letter1) ;
      letter2 = tolower(letter2) ;
      }
   return m_one2one[(unsigned)letter1][(unsigned)letter2] > 0.0f ;
}

//----------------------------------------------------------------------------

size_t CognateData::cognateLetters(const char* str1, const char* str2, size_t& len1, size_t& len2) const
{
   (void)str1; (void)str2; (void)len1; (void)len2;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

static CogScoreInfo** alloc_score_buffer(size_t rows, size_t columns)
{
   auto buffer = new CogScoreInfo*[rows] ;
   if (!buffer)
      return buffer ;
   for (size_t i = 0 ; i < rows ; ++i)
      buffer[i] = new CogScoreInfo[columns] ;
   return buffer ;
}

//----------------------------------------------------------------------------

static void free_score_buffer(CogScoreInfo** buffer, size_t rows)
{
   if (!buffer)
      return;
   for (size_t i = 0 ; i < rows ; ++i)
      delete[] buffer[i] ;
   delete buffer ;
   return ;
}

//----------------------------------------------------------------------------

static float score_exact_matches(const char* word1, size_t len1, const char* word2, size_t len2,
   CogScoreInfo** score_buf, size_t rows)
{
   // initialize the first row of the scoring matrix
   for (size_t col = 0 ; col < len2 ; ++col)
      {
      score_buf[0][col].init(0.0f,0,1) ;
      }
   // now iterate each row, checking for best choice of substitute/delete/insert
   for (size_t row = 1 ; row < len1 ; ++row)
      {
      score_buf[row%rows][0] = score_buf[(row-1)%rows][0] ;
      for (size_t col = 1 ; col < len2 ; ++col)
	 {
	 float ins = score_buf[row%rows][col-1] ;
	 float del = score_buf[(row-1)%rows][col] ;
	 float subst = score_buf[(row-1)%rows][col-1] + (word1[row] == word2[col]) ? 1.0 : 0.0 ;
	 if (ins > del && ins > subst)
	    score_buf[row%rows][col].init(ins,0,1) ;
	 else if (del > ins && del > subst)
	    score_buf[row%rows][col].init(del,1,0) ;
	 else
	    score_buf[row%rows][col].init(subst,1,1) ;
	 }
      }
   return score_buf[(len1-1)%rows][len2-1].score ;
}

//----------------------------------------------------------------------------

float CognateData::score_single_byte(const char* word1, size_t len1, const char* word2, size_t len2,
   CogScoreInfo** score_buf, size_t rows) const
{
   // initialize the first row of the scoring matrix
   score_buf[0][0].init(0.0,0,0) ;
   for (size_t col = 0 ; col < len2 ; ++col)
      {
      score_buf[0][col].init(score_buf[0][col-1] + m_insertion[(unsigned char)word2[col-1]],0,1) ;
      }
   // now iterate each row, checking for best choice of substitute/delete/insert
   for (size_t row = 1 ; row < len1 ; ++row)
      {
      score_buf[row%rows][0].init(score_buf[(row-1)%rows][0] + m_deletion[(unsigned char)word1[row-1]],1,0) ;
      for (size_t col = 1 ; col < len2 ; ++col)
	 {
	 float ins = score_buf[row%rows][col-1] + m_insertion[(unsigned char)word2[col-1]] ;
	 float del = score_buf[(row-1)%rows][col] + m_deletion[(unsigned char)word1[row-1]] ;
	 float subst = score_buf[(row-1)%rows][col-1]
	    + m_one2one[(unsigned char)word1[row]][(unsigned char)word2[col]] ;
	 if (ins > del && ins > subst)
	    score_buf[row%rows][col].init(ins,0,1) ;
	 else if (del > ins && del > subst)
	    score_buf[row%rows][col].init(del,1,0) ;
	 else
	    score_buf[row%rows][col].init(subst,1,1) ;
	 }
      }
   return score_buf[(len1-1)%rows][len2-1].score ;
}

//----------------------------------------------------------------------------

float CognateData::scale_match(size_t srcmatch, size_t trgmatch, size_t srclen, size_t trglen) const
{
   if (m_rel_to_shorter)
      {
      return (srclen < trglen) ? srcmatch : trgmatch ;
      }
   else if (m_rel_to_average)
      {
      //float srcfrac = srclen ? srcmatch / (float)srclen : 1.0 ;
      //float trgfrac = trglen ? trgmatch / (float)trglen : 1.0 ;
      return (srcmatch + trgmatch) / 2.0f ; //FIXME
      }
   else // relative to longer
      {
      return (srclen > trglen) ? srcmatch : trgmatch ;
      }
}

//----------------------------------------------------------------------------

float CognateData::best_match(const char* word1, size_t len1, size_t index1,
   const char* word2, size_t len2, size_t index2,
   size_t& srclen, size_t &trglen) const
{
   size_t best_src = 0 ;
   size_t best_trg = 0 ;
   float best_score = 0.0f;
   if (m_mappings)
      {
      size_t maxsrc = std::min(index1,(size_t)m_mappings->longestKey()) ;
      auto idx = Trie<List*>::ROOT_INDEX ;
      for (size_t i = 1 ; i <= maxsrc ; ++i)
	 {
	 if (!m_mappings->extendKey(idx,(uint8_t)word1[index1-i]))
	    break ;
	 auto n = m_mappings->node(idx) ;
	 if (n->leaf())
	    {
	    const List* targets = n->value() ;
	    if (!targets) continue ;
	    // iterate through list of target strings
	    for (const Object* target : *targets)
	       {
	       if (!target) continue ;
	       const List* targetspec = reinterpret_cast<const List*>(target) ;
	       const String* targetstr = reinterpret_cast<const String*>(targetspec->front()) ;
	       size_t targetlen = targetstr->c_len() ;
	       if (targetlen > index2)
		  continue ;		// too long to match word2
	       const char* trgstr = targetstr->c_str() ;
	       if (memcmp(trgstr,word2+index2-targetlen,targetlen) == 0)
		  {
		  // we found a match, so compute its score, scaling for the
		  //   relative lengths of the match and input strings
		  float rawscore = (float)targetspec->nth(2)->floatValue() ;
		  float score = scale_match(i,targetlen,len1,len2) * rawscore ;
		  if (score > best_score)
		     {
		     best_score = score ;
		     best_src = i ;
		     best_trg = targetlen ;
		     }
		  }
	       }
	    }
	 }
      }
   srclen = best_src ;
   trglen = best_trg ;
   return best_score ;
}

//----------------------------------------------------------------------------

float CognateData::score_general(const char* word1, size_t len1, const char* word2, size_t len2,
   CogScoreInfo** score_buf, size_t rows) const
{
   // initialize the first row of the scoring matrix
   score_buf[0][0].init(0.0f,0,0) ;
   for (size_t col = 1 ; col < len2 ; ++col)
      {
      score_buf[0][col].init(score_buf[0][col-1] + m_insertion[(unsigned char)word2[col-1]],0,1) ;
      }
   // now iterate each row, checking for best choice of substitute/delete/insert
   for (size_t row = 1 ; row < len1 ; ++row)
      {
      score_buf[row%rows][0].init(score_buf[(row-1)%rows][0] + m_deletion[(unsigned char)word1[row-1]],1,0) ;
      for (size_t col = 1 ; col < len2 ; ++col)
	 {
	 float ins = score_buf[row%rows][col-1] + m_insertion[(unsigned char)word2[col-1]] ;
	 float del = score_buf[(row-1)%rows][col] + m_deletion[(unsigned char)word1[row-1]] ;
	 float subst = score_buf[(row-1)%rows][col-1]
	    + m_one2one[(unsigned char)word1[row]][(unsigned char)word2[col]] ;
	 size_t srclen ;
	 size_t trglen ;
	 float subst2 = best_match(word1,len1,row,word2,len2,col,srclen,trglen) ;
	 if (ins > del && ins > subst && ins > subst2)
	    score_buf[row%rows][col].init(ins,0,1) ;
	 else if (del > ins && del > subst && del > subst2)
	    score_buf[row%rows][col].init(del,1,0) ;
	 else if (subst >= ins && subst >= del && subst >= subst2)
	    score_buf[row%rows][col].init(subst,1,1) ;
	 else
	    score_buf[row%rows][col].init(subst2,srclen,trglen) ;
	 }
      }
   return score_buf[(len1-1)%rows][len2-1].score ;
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

static CognateAlignment* extract_alignment(const CogScoreInfo* const* score_buf, size_t srclen, size_t trglen)
{
   // first, count the number of segments
   size_t cur_src { srclen }, cur_trg { trglen } ;
   size_t count { 0 } ;
   while (cur_src > 0 && cur_trg > 0)
      {
      ++count ;
      const CogScoreInfo &info = score_buf[cur_src][cur_trg] ;
      cur_src -= info.srclen ;
      cur_trg -= info.trglen ;
      }
   // allocate the alignment array
   CognateAlignment* align = new CognateAlignment[count+1] ;
   align[count].init(0,0) ;		// add the terminating sentinel
   if (count > 0)
      {
      cur_src = srclen ;
      cur_trg = trglen ;
      while (cur_src > 0 && cur_trg > 0)
	 {
	 const CogScoreInfo &info = score_buf[cur_src][cur_trg] ;
	 align[--count].init(info.srclen,info.trglen) ;
	 }
      }
   return align ;
}

//----------------------------------------------------------------------------

double CognateData::score(const char* word1, const char* word2, bool exact_letter_match_only,
   CognateAlignment** align) const
{
   if (!word1 || !word2 || (!*word1 && !*word2))
      return 0.0 ;
   size_t rows = align ? strlen(word1) + 1 : longestSource() + 1 ;
   auto score_buf = alloc_score_buffer(rows,strlen(word2)+1) ;
   float cogscore ;
   size_t len1 = strlen(word1) ;
   size_t len2 = strlen(word2) ;
   if (exact_letter_match_only)
      {
      cogscore = score_exact_matches(word1,len1,word2,len2,score_buf,rows) ;
      }
   else if (!m_mappings)
      {
      cogscore = score_single_byte(word1,len1,word2,len2,score_buf,rows) ;
      }
   else
      {
      cogscore = score_general(word1,len1,word2,len2,score_buf,rows) ;
      }
   if (align)
      {
      // accumulate alignment info from the source/target lengths recorded in score_buf
      *align = extract_alignment(score_buf,len1,len2) ;
      }
   free_score_buffer(score_buf,rows) ;
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
