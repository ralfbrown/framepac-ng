//FIXME FIXME

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
   else if (item->numberp())
      {
      if (item->floatp())
	 outfp << item->floatValue() ;
      else
	 outfp << item->intValue() ;
      }
   else if (item->symbolp() || item->stringp())
      {
      write_json_string(outfp,item->printableName()) ;
      }
   else if (item->consp())
      {
      List *list = (List*)item ;
      if (list->first() && list->first()->consp()
	  && ((List*)list->first())->first()
	  && ((List*)list->first())->first()->stringp())
	 {
	 write_json(outfp,list,indent+1,true) ;
	 }
      else
	 {
	 // it's a plain list, not a map
	 outfp << "[" ;
	 for ( ; list ; list = list->rest())
	    {
	    write_json_item(outfp,list->first(),indent) ;
	    if (list->rest())
	       outfp << "," ;
	    }
	 outfp << "]" ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

void write_json(CFile& outfp, const List *json, int indent, bool recursive)
{
   if (outfp.eof())
      return  ;
   if (!json || !json->consp())
      {
      write_json_item(outfp,json,indent) ;
      return ;
      }
   if (!recursive)
      outfp.printf("%*s{\n",(2*indent),"") ;
   else
      outfp << "{\n" ;
   indent++ ;
   for ( ; json ; json = json->rest())
      {
      List *field = (List*)json->first() ;
      String *fieldname = (String*)field->first() ;
      List *value = field->rest() ;
      outfp.printf("%*s\"%s\": ",(2*indent)," ",fieldname->printableName()) ;
      if (value->simplelistlength() > 1)
	 {
	 outfp << "[" ;
	 for ( ; value ; value = value->rest())
	    {
	    write_json_item(outfp,value->first(),indent) ;
	    if (value->rest())
	       outfp << "," ;
	    }
	 outfp << "]," ;
	 }
      else
	 {
	 write_json_item(outfp,value->first(),indent) ;
	 outfp << "," ;
	 }
      outfp << '\n' ;
      }
   indent-- ;
   outfp.printf("%*s}",(2*indent),"") ;
//   if (!recursive)
//      outfp << '\n' ;
   return ;
}

