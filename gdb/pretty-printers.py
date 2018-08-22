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

RECURSIVE_CALL = False

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
        RECURSIVE_CALL = True
        count = 0
        members = self.val['m_array']
        arrsize = int(self.val['m_size'])
        while count < arrsize:
            yield str(count), self.safe_dereference(members[count])
            count = count + 1
        RECURSIVE_CALL = False
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
        return 'BitVector({}/{})'.format(int(str(self.val['m_size'])),int(str(self.val['m_capacity'])))

    def display_hint(self):
        return 'number'

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
        table = self.val['m_table']['v']
        entries = table['m_entries']
        objptr = gdb.lookup_type('Fr::Object').pointer()
        size = int(table['m_size'])
        for i in range(size):
            key = self.safe_dereference(entries[i]['m_key']['v'])
            try:
                x = str(key)
                yield 'key', key
                yield 'val', self.safe_dereference(entries[i]['m_value'][0].cast(objptr))
            except:
                # empty slots have a key pointing at 0xfff..fff, which generates an exception when we try
                #   to convert it to a string.  Skip empy slots.
                pass
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
        if self.is_empty_list():
            if RECURSIVE_CALL:
                return '()'
            else:
                return 'empty List'
        RECURSIVE_CALL = True
        return 'List'

    def display_hint(self):
        return 'array'

    def make_children(self):
        global RECURSIVE_CALL
        if self.no_children:
            return
        count = 0
        while count < 64 and not self.is_empty_list():
            head = self.safe_dereference(self.val['m_item'])
            self.curraddr = self.nextaddr
            self.val = self.val['m_next'].dereference()
            yield str(count),head
            count = count + 1
        if not self.is_empty_list():
            yield str(count),gdb.Value('...')
        RECURSIVE_CALL = False
        return

    def children(self):
        return self.make_children()

##########################################################################

class FrRefArrayPrinter(FrArrayPrinter):
    "Print a Fr::RefArray object"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.arrtype = 'Ref'

##########################################################################

class FrStringPrinter(gdb.printing.PrettyPrinter):
    "Print a Fr::String object"
    enabled = True

    def __init__(self, val):
        self.val = val

    def escape_byte(self, b):
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
    
    def to_string(self):
        packedptr = self.val['m_buffer']['m_pointer'] ;
        len = (int(packedptr) >> 48) & 0xFFFF
        ptr = int(packedptr) & 0x0000FFFFFFFFFFFF
        if RECURSIVE_CALL:
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
        if RECURSIVE_CALL:
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
        self.members = self.val['m_values']

    def to_string(self):
        keystr = self.safe_dereference(self.val['m_key'])
        lblstr = self.safe_dereference(self.val['m_label'])
        return '{}Vector({}/{} key={} label={})'.format(self.vectype,self.vecsize,int(self.val['m_capacity']),
                                                        keystr,lblstr)

    def display_hint(self):
        return 'array'

    def make_children(self):
        global RECURSIVE_CALL
        RECURSIVE_CALL = True
        count = 0
        while count < self.vecsize:
            yield str(count),self.members[count]
            count = count + 1
        RECURSIVE_CALL = False
        return
    
    def children(self):
        return self.make_children()

##########################################################################

class FrSparseVectorPrinter(FrVectorPrinter):
    "Print a Fr::SparseVector<> object"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.vectype = 'Sparse'
        self.vecsize = int(self.val['m_size'])
        self.members = self.val['m_values']
        self.indices = self.val['m_indices']

    def display_hint(self):
        return 'map'

    def make_children(self):
        global RECURSIVE_CALL
        RECURSIVE_CALL = True
        count = 0
        while count < self.vecsize:
            yield str(count),self.indices[count]
            yield str(count),self.members[count]
            count = count + 1
        RECURSIVE_CALL = False
        return
    
    def children(self):
        return self.make_children()

##########################################################################

class FrObjectPrinter(gdb.printing.PrettyPrinter):
    "Print an arbitrary Fr::Object object"
    enabled = True

    def read_ASCIZ(self, addr):
        charptr = gdb.lookup_type('char').pointer()
        strptr = gdb.Value(addr).cast(charptr)
        try:
            return strptr.string()
        except:
            return '(invalid)'

    def get_typename(self, addr):
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

    def __init__(self, val):
        self.val = val
        self.printer = None
        self.objtype = self.get_typename(val.address)
        if self.objtype == 'String':
            strptr = gdb.lookup_type('Fr::String')
            self.printer = FrStringPrinter(val.cast(strptr))
        elif self.objtype == 'Symbol':
            symptr = gdb.lookup_type('Fr::Symbol')
            self.printer = FrSymbolPrinter(val.cast(symptr))
        elif self.objtype == 'Array':
            arrptr = gdb.lookup_type('Fr::Array')
            self.printer = FrArrayPrinter(val.cast(arrptr))
        elif self.objtype == 'RefArray':
            arrptr = gdb.lookup_type('Fr::RefArray')
            self.printer = FrRefArrayPrinter(val.cast(arrptr))
        elif self.objtype == 'BitVector':
            bvptr = gdb.lookup_type('Fr::BitVector')
            self.printer = FrBitvectorPrinter(val.cast(bvptr))
        elif self.objtype == 'Float':
            fltptr = gdb.lookup_type('Fr::Float')
            self.printer = FrFloatPrinter(val.cast(fltptr))
        elif self.objtype == 'Integer':
            intptr = gdb.lookup_type('Fr::Integer')
            self.printer = FrIntegerPrinter(val.cast(intptr))
        elif self.objtype == 'ObjHashTable':
            htptr = gdb.lookup_type('Fr::HashTable<Fr::Object*, Fr::Object*>')
            self.printer = FrHashTablePrinter(val.cast(htptr))
        elif self.objtype == 'SymHashTable':
            htptr = gdb.lookup_type('Fr::HashTable<Fr::Symbol*, Fr::Object*>')
            self.printer = FrHashTablePrinter(val.cast(htptr))
        elif self.objtype == 'List':
            lstptr = gdb.lookup_type('Fr::List')
            self.printer = FrListPrinter(val.cast(lstptr))
        elif self.objtype == 'SparseVector_u32flt':
            svptr = gdb.lookup_type('Fr::SparseVector<unsigned int, float>')
            self.printer = FrSparseVectorPrinter(val.cast(svptr))

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

    def to_string(self):
        ptr = self.val['m_object']
        return '{} @ {}'.format(self.typestr,ptr.address)

    def display_hint(self):
        return 'struct'

    def children(self):
        ptr = self.safe_dereference(self.val['m_object'])
        return [('obj',ptr)]

##########################################################################

class FrScopedObjectPrinter(FrPtrObjectPrinter):
    "Print the object pointed at by a ScopedObject"
    enabled = True

    def __init__(self, val):
        self.val = val
        self.typestr = 'ScopedObject'
        
##########################################################################

def build_pretty_printer():
   pp = gdb.printing.RegexpCollectionPrettyPrinter("FramepaC-ng")
   pp.add_printer('std::mutex', '^std::mutex$', StdMutexPrinter)
   pp.add_printer('Fr::Atomic', '^Fr::Atomic<', FrAtomicPrinter)
   pp.add_printer('Fr::Array', '^Fr::Array$', FrArrayPrinter)
   pp.add_printer('Fr::BitVector', '^Fr::BitVector$', FrBitvectorPrinter)
   pp.add_printer('Fr::Float', '^Fr::Float$', FrFloatPrinter)
   pp.add_printer('Fr::HashTable', '^Fr::HashTable<', FrHashTablePrinter)
   pp.add_printer('Fr::Integer', '^Fr::Integer$', FrIntegerPrinter)
   pp.add_printer('Fr::List', '^Fr::List$', FrListPrinter)
   pp.add_printer('Fr::Object', '^Fr::Object *$', FrObjectPrinter)
   pp.add_printer('Fr::RefArray', '^Fr::RefArray$', FrRefArrayPrinter)
   pp.add_printer('Fr::SparseVector', '^Fr::SparseVector<', FrSparseVectorPrinter)
   pp.add_printer('Fr::String', '^Fr::String$', FrStringPrinter)
   pp.add_printer('Fr::Symbol', '^Fr::Symbol$', FrSymbolPrinter)
   pp.add_printer('Fr::Vector', '^Fr::Vector<', FrVectorPrinter)
   pp.add_printer('Fr::ObjectPtr', '^Fr::Ptr<Object>$', FrPtrObjectPrinter)
   pp.add_printer('Fr::ScopedObject', '^Fr::ScopedObject<', FrScopedObjectPrinter)
   pp.add_printer('TermVectorSparse', 'TermVectorSparse', FrSparseVectorPrinter)
   return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(),build_pretty_printer())

### end of file ###
