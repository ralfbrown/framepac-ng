##########################################################################
## Version 0.09, last edit 2018-08-19					##
##	by Ralf Brown <ralf@cs.cmu.edu>					##
##									##
## (c) Copyright 2018 Carnegie Mellon University			##
##	This program may be redistributed and/or modified under the	##
##	terms of the GNU General Public License, version 3, or an	##
##	alternative license agreement as detailed in the accompanying	##
##	file LICENSE.  You should also have received a copy of the	##
##	GPL (file COPYING) along with this program.  If not, see	##
##	http://www.gnu.org/licenses/					##
##									##
##	This program is distributed in the hope that it will be		##
##	useful, but WITHOUT ANY WARRANTY; without even the implied	##
##	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR		##
##	PURPOSE.  See the GNU General Public License for more details.	##
##########################################################################

import gdb.printing

RECURSIVE_CALL = 0

##########################################################################

class StdMutexPrinter(gdb.printing.PrettyPrinter):
    "Print a std::mutex"
    enabled = True

    def __init__(self, val):
        self.val = val

    def to_string(self):
        data = self.val['_M_mutex']['__data']
        count = data['__count']
        owner = data['__owner']
        nusers = int(data['__nusers'])
        spins = int(data['__spins'])
        return "mutex(own={},cnt={},users={},spins={})".format(owner,count,nusers,spins)

    def display_hint(self):
        return 'struct'

##########################################################################

class FrPrinter(gdb.printing.PrettyPrinter):
    "A parent class to store utility functions"

    @staticmethod
    def safe_dereference(val):
        if val and val.address:
            addr = str(val.address).split(' ')
            if int(addr[0],16) != 0:
                try:
                    return val.dereference()
                except:
                    return '@'+addr[0]
        return 'NULL'

    @staticmethod
    def escape_byte(b):
        value = ord(b)
        if value == ord('\\'):
            return '\\\\'
        if value == 9 or value >= 32:
            return chr(value)
        if value == 0:
            return '\\0'
        if value == 10:
            return '\\n'
        if value == 13:
            return '\\r'
        if value == 27:
            return '\\e'
        return "^" + chr(value+64)
        
    def collect_string(self, addr, len, quote = True):
        ellipsis = ''
        if len > 64:
            len = 61
            ellipsis = '...'
        try:
            bytes = gdb.selected_inferior().read_memory(addr,len)
        except gdb.MemoryError:
            return '(unreadable memory)'
        if quote:
            chars = ['"'] + [self.escape_byte(b) for b in bytes] + [ellipsis, '"']
        else:
            chars = [self.escape_byte(b) for b in bytes] + [ellipsis]
            if set(''.join(chars)) - set('ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_'):
                chars = ['|'] + chars + ['|']
        return ''.join(chars)

    @staticmethod
    def read_ASCIZ(addr):
        charptr = gdb.lookup_type('char').pointer()
        strptr = gdb.Value(addr).cast(charptr)
        try:
            return strptr.string()
        except:
            return '(memerr)'

    @staticmethod
    def get_typename(addr):
        if addr is None:
            return '(unknown)'
        # str(addr) is the address of the value
        addr = int(str(addr),16)
        vmt = addr & 0xfffffffff000
        charptrptr = gdb.lookup_type('char').pointer().pointer().pointer()
        try:
            vmtaddr = gdb.Value(vmt).cast(charptrptr).dereference().dereference()
            return vmtaddr.string()
        except:
            return '(invalid)'

##########################################################################

class FrAtomicPrinter(gdb.printing.PrettyPrinter):
    "Print a Fr::Atomic object"
    enabled = True

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "atm({})".format(self.val['v'])

    def display_hint(self):
        return 'integer'

##########################################################################

class FrArrayPrinter(FrPrinter):
    "Print a Fr::Array object"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.arrtype = ''

    def to_string(self):
        return "{}Array({}/{})".format(self.arrtype,int(self.val['m_size']),int(self.val['m_alloc']))

    def display_hint(self):
        return 'array'

    def make_children(self):
        global RECURSIVE_CALL
        RECURSIVE_CALL += 1
        count = 0
        members = self.val['m_array']
        arrsize = int(self.val['m_size'])
        while count < arrsize:
            yield str(count), self.safe_dereference(members[count])
            count = count + 1
        RECURSIVE_CALL -= 1
        return

    def children(self):
        return self.make_children()

##########################################################################

class FrBitvectorPrinter(gdb.printing.PrettyPrinter):
    "Print a Fr::BitVector object"
    enabled = True

    def __init__(self, val):
        self.val = val
        
    def to_string(self):
        size = int(str(self.val['m_size']))
        if size > 0:
            bits = '{:b}'.format(int(str(self.val['m_bits'][0])))[::-1]
        return 'BitVector({}/{}:{})'.format(size,int(str(self.val['m_capacity'])),bits)

    def display_hint(self):
        return 'number'

##########################################################################

class FrCharPtrPrinter(FrPrinter):
    "Print a CharPtr (NewPtr<char>) object"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.arrtype = 'Ref'

    def to_string(self):
        return 'CharPtr'

    def children(self):
        return [('str',self.val['m_string'])]

##########################################################################

class FrIntegerPrinter(gdb.printing.PrettyPrinter):
    "Print a Fr::Integer object"
    enabled = True

    def __init__(self, val):
        self.val = val
        
    def to_string(self):
        return str(int(str(self.val['m_value']),16))

    def display_hint(self):
        return 'number'

##########################################################################

class FrFloatPrinter(gdb.printing.PrettyPrinter):
    "Print a Fr::Float object"
    enabled = True

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return self.val['m_value']

    def display_hint(self):
        return 'number'

##########################################################################

class FrHashTablePrinter(FrPrinter):
    "Print a Fr::HashTable object"
    enabled = True

    def __init__(self, val):
        self.val = val

    def to_string(self):
        table = self.val['m_table']['v']
        return 'HashTable({}) @ {}'.format(table['m_size'],self.val.address)

    def display_hint(self):
        return 'map'

    def make_children(self):
        global RECURSIVE_CALL
        RECURSIVE_CALL += 1
        table = self.val['m_table']['v']
        entries = table['m_entries']
        objptr = gdb.lookup_type('Fr::Object').pointer()
        size = int(table['m_size'])
        for i in range(size):
            try:
                # get the key; no need for safe_dereference() since were in a try block anyway
                key = entries[i]['m_key']['v'].dereference()
                x = str(key)  # causes an exception if the slot is empty
                yield 'key', key
                yield 'val', self.safe_dereference(entries[i]['m_value'][0].cast(objptr))
            except:
                # empty slots have a key pointing at 0xfff..fff, which generates an exception when we try
                #   to convert it to a string.  Skip empy slots.
                pass
        RECURSIVE_CALL -= 1
        return

    def children(self):
        return self.make_children()

##########################################################################

class FrListPrinter(FrPrinter):
    "Print a Fr::List object"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.voidptr = gdb.lookup_type('void').pointer()
        self.curraddr =  str(val.address)
        self.no_children = self.is_empty_list()
        
    def is_empty_list(self):
        self.nextaddr = str(self.val['m_next'].cast(self.voidptr))
        return self.curraddr == self.nextaddr
    
    def to_string(self):
        global RECURSIVE_CALL
        if self.no_children:
            if RECURSIVE_CALL > 0:
                return '()'
            else:
                return 'empty List'
        return 'List'

    def display_hint(self):
        return 'array'

    def children(self):
        global RECURSIVE_CALL
        if self.no_children:
            return
        RECURSIVE_CALL += 1
        count = 0
        while count < 64 and not self.is_empty_list():
            head = self.safe_dereference(self.val['m_item'])
            self.curraddr = self.nextaddr
            self.val = self.val['m_next'].dereference()
            yield str(count),head
            count = count + 1
        if not self.is_empty_list():
            yield str(count),gdb.Value('...')
        RECURSIVE_CALL -= 1
        return

##########################################################################

class FrListBuilderPrinter(FrPrinter):
    "Print the contents of a ListBuilder"
    enabled = True

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return 'ListBuilder @ {}'.format(self.val.address)

    def children(self):
        global RECURSIVE_CALL
        RECURSIVE_CALL += 1
        yield 'l', self.safe_dereference(self.val['m_list'])
        RECURSIVE_CALL -= 1
        return

##########################################################################

class FrRefArrayPrinter(FrArrayPrinter):
    "Print a Fr::RefArray object"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.arrtype = 'Ref'

##########################################################################

class FrStringPrinter(FrPrinter):
    "Print a Fr::String object"
    enabled = True

    def __init__(self, val):
        self.val = val

    def to_string(self):
        packedptr = self.val['m_buffer']['m_pointer'] ;
        len = (int(packedptr) >> 48) & 0xFFFF
        ptr = int(packedptr) & 0x0000FFFFFFFFFFFF
        if RECURSIVE_CALL > 0:
            return self.collect_string(ptr,len)
        else:
            return "String({},{})".format(len,self.collect_string(ptr,len))

    def display_hint(self):
        return 'array'

##########################################################################

class FrSymbolPrinter(FrStringPrinter):
    "Print a Fr::Symbol object"
    enabled = True

    def __init__(self, val):
        self.val = val

    def to_string(self):
        packedptr = self.val['m_buffer']['m_pointer']
        propptr = self.val['m_properties']['m_pointer']
        len = (int(packedptr) >> 48) & 0xFFFF
        ptr = int(packedptr) & 0x0000FFFFFFFFFFFF
        propflags = (int(propptr) >> 48) & 0x0F
        symtab = (int(propptr) >> 52) & 0xFFF
        if RECURSIVE_CALL > 0:
            return self.collect_string(ptr,len,False)
        else:
            return "Symbol({},{},{})".format(self.collect_string(ptr,len),symtab,propflags)

    def display_hint(self):
        return 'array'

##########################################################################

class FrVectorPrinter(FrPrinter):
    "Print a Fr::Vector<> object"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.vectype = ''
        self.vecsize = int(self.val['m_size'])
        self.indices = None

    def to_string(self):
        global RECURSIVE_CALL
        keystr = self.safe_dereference(self.val['m_key'])
        lblstr = self.safe_dereference(self.val['m_label'])
        if RECURSIVE_CALL > 0:
            return '{}Vector({}/{} key={} label={}) @ {}'.format(self.vectype,self.vecsize,int(self.val['m_capacity']),
                                                            keystr,lblstr,self.val.address)
        else:
            return '{}Vector({}/{} key={} label={})'.format(self.vectype,self.vecsize,int(self.val['m_capacity']),
                                                            keystr,lblstr)

    def display_hint(self):
        return 'array'

    def make_children(self):
        global RECURSIVE_CALL
        RECURSIVE_CALL += 1
#        members = self.val['m_values']
        members = self.safe_dereference(self.val['m_values'])
        count = 0
        while count < self.vecsize:
            if self.indices:
                yield str(count), self.indices[count]
            yield str(count), members[count]
            count = count + 1
        RECURSIVE_CALL -= 1
        return
    
    def children(self):
        global RECURSIVE_CALL
        if RECURSIVE_CALL > 0:
            return []
        return self.make_children()

##########################################################################

class FrSparseVectorPrinter(FrVectorPrinter):
    "Print a Fr::SparseVector<> object"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.vectype = 'Sparse'
        self.vecsize = int(self.val['m_size'])
        self.indices = self.val['m_indices']

    def display_hint(self):
        return 'map'
    
##########################################################################

class FrObjectPrinter(FrPrinter):
    "Print an arbitrary Fr::Object object"
    enabled = True
    printers = { 'String'   : ('Fr::String', FrStringPrinter),
                 'Array'    : ('Fr::Array', FrArrayPrinter),
                 'BitVector': ('Fr::BitVector', FrBitvectorPrinter),
                 'Float'    : ('Fr::Float', FrFloatPrinter),
                 'Integer'  : ('Fr::Integer', FrIntegerPrinter),
                 'List'     : ('Fr::List', FrListPrinter),
                 'RefArray' : ('Fr::RefArray', FrRefArrayPrinter),
                 'Symbol'   : ('Fr::Symbol', FrSymbolPrinter),
                 'ObjHashTable' : ('Fr::HashTable<Fr::Object*, Fr::Object*>', FrHashTablePrinter),
                 'ObjCountHashTable' : ('Fr::HashTable<Fr::Object*, unsigned long>', FrHashTablePrinter),
                 'SymHashTable' : ('Fr::HashTable<Fr::Symbol*, Fr::Object*>', FrHashTablePrinter),
                 'SymCountHashTable' : ('Fr::HashTable<Fr::Symbol*, unsigned long>', FrHashTablePrinter),
                 'HashTable_u32u32' : ('Fr::HashTable<unsigned int, unsigned int>', FrHashTablePrinter),
                 'SparseVector_u32flt': ('Fr::SparseVector<unsigned int, float>', FrSparseVectorPrinter),
                 'WcTermVectorSparse' : ('Fr::SparseVector<unsigned int, float>', FrSparseVectorPrinter)
                 }

    def __init__(self, val):
        self.val = val
        self.printer = None
        self.objtype = self.get_typename(val.address)
        if self.objtype in self.printers:
            typestr, pr = self.printers[self.objtype]
            datatype = gdb.lookup_type(typestr)
            self.printer = pr(val.cast(datatype))
        return

    def to_string(self):
        if self.printer:
            return self.printer.to_string()
        return "{} @ {}".format(self.objtype,self.val.address)

    def display_hint(self):
        if self.printer:
            return self.printer.display_hint()
        return 'string'

    def children(self):
        if self.printer and getattr(self.printer, 'children', None):
            return self.printer.children()
        return []  ## no children

##########################################################################

class FrPtrObjectPrinter(FrPrinter):
    "Print the object pointed at by an ObjectPtr"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.typestr = '->Object'
        self.label = 'obj'

    def to_string(self):
        ptr = self.val['m_object']
        return '{} @ {}'.format(self.typestr,ptr.address)

    def display_hint(self):
        return 'struct'

    def children(self):
        ptr = self.safe_dereference(self.val['m_object'])
        return [(self.label,ptr)]

##########################################################################

class FrPtrArrayPrinter(FrPtrObjectPrinter):
    "Print the object pointed at by an ArrayPtr"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.typestr = '->Array'
        self.label = 'arr'

##########################################################################

class FrPtrListPrinter(FrPtrObjectPrinter):
    "Print the object pointed at by a ListPtr"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.typestr = '->List'
        self.label = 'l'

##########################################################################

class FrPtrStringPrinter(FrPtrObjectPrinter):
    "Print the object pointed at by a StringPtr"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.typestr = '->String'
        self.label = 's'

##########################################################################

class FrScopedObjectPrinter(FrPtrObjectPrinter):
    "Print the object pointed at by a ScopedObject"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.typestr = 'ScopedObject'
        self.label = 'obj'

##########################################################################

def build_pretty_printer():
   pp = gdb.printing.RegexpCollectionPrettyPrinter("FramepaC-ng")
   pp.add_printer('std::mutex', '^std::mutex$', StdMutexPrinter)
   pp.add_printer('Fr::Atomic', '^Fr::Atomic<', FrAtomicPrinter)
   pp.add_printer('Fr::Array', '^Fr::Array$', FrArrayPrinter)
   pp.add_printer('Fr::ArrayPtr', '^Fr::Ptr<Array>$', FrPtrArrayPrinter)
   pp.add_printer('Fr::BitVector', '^Fr::BitVector$', FrBitvectorPrinter)
   pp.add_printer('Fr::CharPtr', '^Fr::NewPtr<char>$', FrCharPtrPrinter)
   pp.add_printer('Fr::Float', '^Fr::Float$', FrFloatPrinter)
   pp.add_printer('Fr::HashTable', '^Fr::HashTable<.*>$', FrHashTablePrinter)
   pp.add_printer('Fr::Integer', '^Fr::Integer$', FrIntegerPrinter)
   pp.add_printer('Fr::List', '^Fr::List$', FrListPrinter)
   pp.add_printer('Fr::ListBuilder', '^Fr::ListBuilder$', FrListBuilderPrinter)
#   pp.add_printer('Fr::ListPtr', '^Fr::Ptr<List>$', FrPtrListPrinter)
   pp.add_printer('Fr::ListPtr', '^Fr::ListPtr$', FrPtrListPrinter)
   pp.add_printer('Fr::Object', '^Fr::Object \*$', FrObjectPrinter)
   pp.add_printer('Fr::RefArray', '^Fr::RefArray$', FrRefArrayPrinter)
   pp.add_printer('Fr::SparseVector', '^Fr::SparseVector<', FrSparseVectorPrinter)
   pp.add_printer('Fr::String', '^Fr::String$', FrStringPrinter)
   pp.add_printer('Fr::StringPtr', '^Fr::Ptr<String>$', FrPtrStringPrinter)
   pp.add_printer('Fr::Symbol', '^Fr::Symbol$', FrSymbolPrinter)
   pp.add_printer('Fr::Vector', '^Fr::Vector<', FrVectorPrinter)
   pp.add_printer('Fr::ObjectPtr', '^Fr::Ptr<Object>$', FrPtrObjectPrinter)
   pp.add_printer('Fr::ScopedObject', '^Fr::ScopedObject<', FrScopedObjectPrinter)
   pp.add_printer('TermVectorSparse', 'TermVectorSparse', FrSparseVectorPrinter)
   return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(),build_pretty_printer())

### end of file ###
