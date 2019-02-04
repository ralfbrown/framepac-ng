/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.13, last edit 2019-02-04					*/
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

#include "framepac/spelling.h"

namespace Fr
{

/************************************************************************/
/*	Global data for this module					*/
/************************************************************************/

static const char* QWERTY[] =
   {
   "`1234567890-= ",
   " qwertyuiop[]\\",
   " asdfghjkl;'  ",
   " zxcvbnm,./   "
   } ;

//TODO: fixup with umlauts at right side
static const char* QWERTZ[] =
   {
   "`1234567890B' ",
   " qwertzuiopU* ",
   " asdfghjklOA' ",
   " yxcvbnm;:-   "
   } ;

static const char* AZERTY[] =
   {
   "`1234567890-= ",
   " azertyuiop^$ ",
   " qsdfghjklm%* ",
   "<wxcvbn?./!   "
   } ;

static const char* DVORAK[] =
   {
   "`1234567890[] ",
   " ',.pyfgcrl/=\\",
   " aoeuidhtns-  ",
   " ;qjkxbmwvz   "
   } ;

static const char* COLEMAK[] =
   {
   "`1234567890-= ",
   " qwfpgjluy;[]\\",
   " arstdhneio'  ",
   " zxcvbkm,./   "
   } ;


/************************************************************************/
/*	Methods for class CognateData					*/
/************************************************************************/

static bool get_key(char key, char* str, char* upper)
{
   if (key == ' ')
      {
      str[0] = '\0' ;			// no such key
      upper[0] = '\0' ;
      }
   else if (key == 'B')
      {
      strcpy(str,"Ã") ;		// German esszett
      strcpy(upper,"SS") ;
      }
   else if (key == 'A')
      {
      strcpy(str,"Ã¤") ;		// a-umlaut
      strcpy(upper,"Ã") ;
      }
   else if (key == 'O')
      {
      strcpy(str,"Ã¶") ;		// o-umlaut
      strcpy(upper,"Ã") ;
      }
   else if (key == 'U')
      {
      strcpy(str,"Ã¼") ;		// u-umlaut
      strcpy(upper,"Ã") ;
      }
   else
      {
      str[0] = key ;
      str[1] = '\0' ;
      upper[0] = key ;
      upper[1] = '\0' ;
      if (islower(key))
	 {
	 upper[0] = toupper(key) ;
	 return true ;
	 }
      }
   return false ;
}

//----------------------------------------------------------------------

static void add_near_neighbors(CognateData* cd, const char* line1, const char* line2, double score)
{
   size_t len1 = strlen(line1) ;
   char str1[8] ;
   char str1up[8] ;
   char str2[8] ;
   char str2up[8] ;
   for (size_t i = 0 ; i < len1 ; ++i)
      {
      bool letter = get_key(line1[i],str1,str1up) ;
      if (!*str1)
	 continue ;
      if (i > 0)
	 {
	 get_key(line1[i-1],str2,str2up) ;
	 if (*str2)
	    {
	    cd->setCognateScoring(str1,str2,score) ;
	    if (letter)
	       cd->setCognateScoring(str1up,str2up,score) ;
	    }
	 }
      if (i + 1 < len1)
	 {
	 get_key(line1[i+1],str2,str2up) ;
	 if (*str2)
	    {
	    cd->setCognateScoring(str1,str2,score) ;
	    if (letter)
	       cd->setCognateScoring(str1up,str2up,score) ;
	    }
	 }
      get_key(line2[i],str2,str2up) ;
      if (*str2)
	 {
	 cd->setCognateScoring(str1,str2,score) ;
	 if (letter)
	    cd->setCognateScoring(str1up,str2up,score) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

static void add_far_neighbors(CognateData* cd, const char* line1, const char* line2, double score)
{
   size_t len1 = strlen(line1) ;
   char str1[8] ;
   char str1up[8] ;
   char str2[8] ;
   char str2up[8] ;
   for (size_t i = 0 ; i < len1 ; ++i)
      {
      bool letter = get_key(line1[i],str1,str1up) ;
      if (!*str1)
	 continue ;
      if (i > 0)
	 {
	 get_key(line2[i-1],str2,str2up) ;
	 if (*str2)
	    {
	    cd->setCognateScoring(str1,str2,score) ;
	    if (letter)
	       cd->setCognateScoring(str1up,str2up,score) ;
	    }
	 }
      if (i + 1 < len1)
	 {
	 get_key(line2[i+1],str2,str2up) ;
	 if (*str2)
	    {
	    cd->setCognateScoring(str1,str2,score) ;
	    if (letter)
	       cd->setCognateScoring(str1up,str2up,score) ;
	    }
	 }
      }
   return ;
}

//----------------------------------------------------------------------

bool CognateData::setAdjacentKeys(const char* layout, double near, double far)
{
   if (!layout || !*layout)
      return false ;
   const char** keys ;
   if (strcasecmp(layout,"QWERTY") == 0)
      keys = QWERTY ;
   else if (strcasecmp(layout,"QWERTZ") == 0)
      keys = QWERTZ ;
   else if (strcasecmp(layout,"AZERTY") == 0)
      keys = AZERTY ;
   else if (strcasecmp(layout,"DVORAK") == 0)
      keys = DVORAK ;
   else if (strcasecmp(layout,"COLEMAK") == 0)
      keys = COLEMAK ;
   else
      return false ;
   add_near_neighbors(this,keys[0],keys[1],near) ;
   add_near_neighbors(this,keys[1],keys[0],near) ;
   add_near_neighbors(this,keys[1],keys[2],near) ;
   add_near_neighbors(this,keys[2],keys[1],near) ;
   add_near_neighbors(this,keys[2],keys[3],near) ;
   add_near_neighbors(this,keys[3],keys[2],near) ;
   if (far > 0.0)
      {
      add_far_neighbors(this,keys[0],keys[1],far) ;
      add_far_neighbors(this,keys[1],keys[0],near) ;
      add_far_neighbors(this,keys[1],keys[2],near) ;
      add_far_neighbors(this,keys[2],keys[1],near) ;
      add_far_neighbors(this,keys[2],keys[3],near) ;
      add_far_neighbors(this,keys[3],keys[2],near) ;
      }
   return true ;
}

//----------------------------------------------------------------------

} // end namespace Fr

// end of file keylayout.C //
