import gdb.printing

class StdMutexPrinter(gdb.printing.PrettyPrinter):
   "Print a std::mutex"

   def __init__(self, val):
       self.val = val

   def to_string(self):
       data = self.val['_M_mutex']['__data']
       count = data['__count']
       owner = data['__owner']
       nusers = data['__nusers']
       spins = data['__spins']
       return "mutex(owner={},count={},users={},spins={})".format(owner,count,nusers,spins)

   def display_hint(self):
       return 'struct'

class FrAtomicPrinter(gdb.printing.PrettyPrinter):
   "Print a Fr::Atomic object"

   def __init__(self, val):
      self.val = val

   def to_string(self):
      return "atomic, value = {}".format(self.val['v'])

   def display_hint(self):
      return 'integer'

class FrArrayPrinter(gdb.printing.PrettyPrinter):
    "Print a Fr::Array object"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "array of size {}, capacity {}".format(int(self.val['m_size']),int(self.val['m_alloc']))

    def display_hint(self):
        return 'array'

    def children(self):
        return [] ##FIXME

class FrListPrinter(gdb.printing.PrettyPrinter):
    "Print a Fr::List object"

    def __init__(self, val, generic=False):
        self.objptr = gdb.lookup_type('Fr::Object').pointer()
        if generic:
            # we were recursively called by FrObjectPrinter, so 'val' is not of the correct type
            self.lst = gdb.lookup_type('Fr::List')
            self.lstptr = self.lst.pointer()
            self.val = val.cast(self.lst)
        else:
            self.val = val

    def to_string(self):
        if not self.val:
            return 'NULL'
        next = self.val['m_next']
        nextptr = next.cast(self.lstptr)
        if nextptr['m_next'] == next:
#        if nextptr == self.val.cast(self.lstptr):
            return "NIL"
        head = self.val['m_item']
        return "List({} ...)".format(FrObjectPrinter(head.cast(self.objptr)).to_string())

    def display_hint(self):
        return 'array'

    def children(self):
        return [] ##FIXME

class FrStringPrinter(gdb.printing.PrettyPrinter):
    "Print a Fr::String object"

    def __init__(self, val):
        self.val = val

    def escape_byte(b):
        value = ord(b)
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
        
    def collect_string(self, addr, len):
        ellipsis = ''
        if len > 64:
            len = 61
            ellipsis = '...'
        try:
            bytes = gdb.selected_inferior().read_memory(addr,len)
        except gdb.MemoryError:
            return '(unreadable memory)'
        chars = ['"'] + [self.escape_byte(b) for b in bytes] + [ellipsis, '"']
        return ''.join(chars)
    
    def to_string(self):
        packedptr = self.val['m_buffer']['m_pointer'] ;
        len = (int(packedptr) >> 48) & 0xFFFF
        ptr = int(packedptr) & 0x0000FFFFFFFFFFFF
        return "String({},{})".format(len,self.collect_string(ptr,len))

    def display_hint(self):
        return 'array'

class FrSymbolPrinter(FrStringPrinter):
    "Print a Fr::Symbol object"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        packedptr = self.val['m_buffer']['m_pointer']
        propptr = self.val['m_properties']['m_pointer']
        len = (int(packedptr) >> 48) & 0xFFFF
        ptr = int(packedptr) & 0x0000FFFFFFFFFFFF
        propflags = (int(propptr) >> 48) & 0x0F
        symtab = (int(propptr) >> 52) & 0xFFF
        return "Symbol({},{},{},{})".format(len,symtab,propflags,self.collect_string(ptr,len))

    def display_hint(self):
        return 'array'

class FrVectorPrinter(gdb.printing.PrettyPrinter):
    "Print a Fr::Vector<> object"

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return self.val['m_size'].string()

    def display_hint(self):
        return 'array'

    def children(self):
        return [] ##FIXME

class FrObjectPrinter(gdb.printing.PrettyPrinter):
    "Print an arbitrary Fr::Object object"

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
            self.printer = FrStringPrinter(val)
        elif self.objtype == 'Symbol':
            self.printer = FrSymbolPrinter(val)
        elif self.objtype == 'Array':
            self.printer = FrArrayPrinter(val)
        elif self.objtype == 'List':
            self.printer = FrListPrinter(val,True)

    def to_string(self):
        if self.printer:
            return self.printer.to_string()
        return "{} object @ {}".format(self.objtype,self.val.address)

    def display_hint(self):
        if self.printer:
            return self.printer.display_hint()
        return 'string'

    def children(self):
        if self.printer and getattr(self.printer, 'children', None):
            return self.printer.children()
        return []  ## no children

def build_pretty_printer():
   pp = gdb.printing.RegexpCollectionPrettyPrinter("FramepaC-ng")
   pp.add_printer('std::mutex', '^std::mutex$', StdMutexPrinter)
   pp.add_printer('Fr::Atomic', '^Fr::Atomic<', FrAtomicPrinter)
   pp.add_printer('Fr::Array', '^Fr::Array$', FrArrayPrinter)
   pp.add_printer('Fr::List', '^Fr::List$', FrListPrinter)
   pp.add_printer('Fr::Object', '^Fr::Object *$', FrObjectPrinter)
   pp.add_printer('Fr::String', '^Fr::String$', FrStringPrinter)
   pp.add_printer('Fr::Symbol', '^Fr::Symbol$', FrSymbolPrinter)
   pp.add_printer('Fr::Vector', '^Fr::Vector<', FrVectorPrinter)
   return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(),build_pretty_printer())

