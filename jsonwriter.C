/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-06					*/
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

#include "framepac/object.h"
#include "framepac/list.h"
#include "framepac/string.h"
#include "framepac/file.h"

//FIXME FIXME

namespace Fr
{

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------

static void write_json_string(CFile& outfp, const char *str)
{
   outfp << '\"' ;
   while (*str)
      {
      if (*str == '\"')
	 outfp << '\\' ;
      else if (*str == '\n')
	 {
	 outfp << "\\n" ;
	 continue ;
	 }
      else if (*str == '\t')
	 {
	 outfp << "\\t" ;
	 continue ;
	 }
      else if (*str == '\r')
	 {
	 outfp << "\\r" ;
	 continue ;
	 }
      outfp << *str++ ;
      }
   outfp << '\"' ;
   return ;
}

//----------------------------------------------------------------------

static void write_json_item(CFile& outfp, const Object* item, int indent)
{
   if (!item)
      outfp.printf("%*snull",(2*indent)," ") ;
   else if (item->isNumber())
      {
      if (item->isFloat())
	 outfp << item->floatValue() ;
      else
	 outfp << item->intValue() ;
      }
   else if (item->isSymbol() || item->isString())
      {
      write_json_string(outfp,item->stringValue()) ;
      }
   else if (item->isList())
      {
      List *list = (List*)item ;
      if (list->front() && list->front()->isList()
	  && ((List*)list->front())->front()
	  && ((List*)list->front())->front()->isString())
	 {
	 outfp.writeJSON(list,indent+1,true) ;
	 }
      else
	 {
	 // it's a plain list, not a map
	 outfp << "[" ;
	 for ( ; list ; list = list->next())
	    {
	    write_json_item(outfp,list->front(),indent) ;
	    if (list->next())
	       outfp << "," ;
	    }
	 outfp << "]" ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

/************************************************************************/
/*	Methods for class CFile						*/
/************************************************************************/

void CFile::writeJSON(const List *json, int indent, bool recursive)
{
   if (eof())
      return  ;
   if (!json || !json->isList())
      {
      write_json_item(*this,json,indent) ;
      return ;
      }
   if (!recursive)
      printf("%*s{\n",(2*indent),"") ;
   else
      (*this) << "{\n" ;
   indent++ ;
   for ( ; json ; json = json->next())
      {
      List *field = (List*)json->front() ;
      String *fieldname = (String*)field->front() ;
      List *value = field->next() ;
      printf("%*s\"%s\": ",(2*indent)," ",fieldname->stringValue()) ;
      if (value->size() > 1)
	 {
	 (*this) << "[" ;
	 for ( ; value ; value = value->next())
	    {
	    write_json_item((*this),value->front(),indent) ;
	    if (value->next())
	       (*this) << "," ;
	    }
	 (*this) << "]," ;
	 }
      else
	 {
	 write_json_item((*this),value->front(),indent) ;
	 (*this) << "," ;
	 }
      (*this) << '\n' ;
      }
   indent-- ;
   printf("%*s}",(2*indent),"") ;
//   if (!recursive)
//      outfp << '\n' ;
   return ;
}


} // end namespace Fr

// end of file jsonwriter.C //

