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

#include "framepac/frame.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

// request explicit instantiation; we declared it "extern" in the header so this
//   will be the only copy of the non-inlined code generated in object modules
template class Allocator<Frame> ;

static const FramepaC::Object_VMT<Frame> frame_vmt ;
Allocator<Frame> Frame::s_allocator(&frame_vmt) ;

/************************************************************************/
/*	Methods for class Frame						*/
/************************************************************************/

Frame::Frame()
{

   return ;
}

//----------------------------------------------------------------------------

Frame::Frame(const Frame *) : Object()
{
   //FIXME

   return ;
}

//----------------------------------------------------------------------------

Frame::Frame(const Frame &) : Object()
{
   //FIXME

   return ;
}

//----------------------------------------------------------------------------

Frame::~Frame()
{

   return ;
}

//----------------------------------------------------------------------------

ObjectPtr Frame::clone_(const Object *)
{

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr Frame::subseq_int(const Object *, size_t start, size_t stop)
{
   (void)start; (void)stop;//FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr Frame::subseq_iter(const Object *, ObjectIter start, ObjectIter stop)
{
   (void)start; (void)stop;//FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

size_t Frame::cStringLength_(const Object *, size_t wrap_at, size_t indent)
{
   (void)wrap_at; (void)indent; //FIXME

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Frame::toCstring_(const Object *, char *buffer, size_t buflen, size_t wrap_at, size_t indent)
{
   (void)buffer; (void)buflen; (void)wrap_at; (void)indent; //FIXME
   //FIXME
   return true ;
}

//----------------------------------------------------------------------------

size_t Frame::size_(const Object *)
{
   return 1 ; //FIXME
}

//----------------------------------------------------------------------------

bool Frame::empty_(const Object *)
{
   return false ; //FIXME
}

//----------------------------------------------------------------------------

Object *Frame::front_(Object *)
{

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

const Object *Frame::front_(const Object *)
{

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

bool Frame::equal_(const Object *, const Object *other)
{
   (void)other; //FIXME

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int Frame::compare_(const Object *, const Object *other)
{
   (void)other; //FIXME

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int Frame::lessThan_(const Object *, const Object *other)
{
   (void)other; //FIXME

   return 0 ; //FIXME
}

/************************************************************************/
/*	Methods for class Slot						*/
/************************************************************************/

/************************************************************************/
/*	Methods for class Facet						*/
/************************************************************************/

//----------------------------------------------------------------------------

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Frame>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::Frame> ;

} // end namespace FramepaC

// end of file frame.C //
