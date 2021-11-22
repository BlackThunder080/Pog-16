import re
import argparse

instructs = {
    'mov': '0x0001',
    'ptr': '0x0002',
    'push': '0x0003',
    'pop': '0x0004',
    'add': '0x0005',
    'sub': '0x0006',
    'mul': '0x0007',
    'div': '0x0008',
    'cmp': '0x0009',
    'jmp': '0x000A',
    'je': '0x000B',
    'jne': '0x000C',
    'jg': '0x000D',
    'jl': '0x000E',
    'call': '0x000F',
    'ret': '0x0010',
    'bit': '0x0011',
    'ldbit': '0x0012',
    'and': '0x0013',
    'or': '0x0014',
    'xor': '0x0015',
    'not': '0x0016',
    'shl': '0x0017',
    'shr': '0x0018',
    'exit': '0xffff'
}

def pad(l, content, width):
    l.extend([content] * (width - len(l)))
    return l

class PeekableStream:
  def __init__(self, iterator):
    self.iterator = iter(iterator)
    self._fill()
  def _fill(self):
    try:
      self.next = next(self.iterator)
    except StopIteration:
      self.next = None
  def move_next(self):
    ret = self.next
    self._fill()
    return ret

def _scan_string(delim, chars):
  ret = ""
  while chars.next != delim:
    c = chars.move_next()
    if c is None:
      raise Exception( \
      "A string ran off the end of the program.")
    ret += c
  chars.move_next()
  return ret

def _scan(first_char, chars, allowed):
  ret = first_char
  p = chars.next
  while p is not None and re.match(allowed, p):
    ret += chars.move_next()
    p = chars.next
  return ret

def lex_line(line):
    if line[0] == '0' and line[1] == 'x':
        return str(int(line[2:], 16))
    elif line[0] == '%':
        return ('0x0000', str(ord(line[1]) - 97))

def lex(chars_iter):
  chars = PeekableStream(chars_iter)
  while chars.next is not None:
    c = chars.move_next()
    if c in ' \n,': pass
        # Ignore white space
    elif c in '%':
        yield ('register', _scan(chars.move_next(), chars, '[a-e]'))
    elif c == '[':
        yield ('address', _scan_string(']', chars))
    elif c == '*':
        yield ('ptr', _scan_string('*', chars))
    elif c == '0' and chars.next == 'x':
        chars.move_next()
        yield ('hex', _scan(chars.move_next(), chars, '[0-9a-f]'))
    elif re.match('[0-9]', c):
        yield ('dec', _scan(c, chars, '[0-9]'))
    elif re.match('[a-zA-Z]', c):
        yield ('instruct', _scan(c, chars, '[a-zA-Z]'))
    else: raise Exception(
        'Unexpected character: \'' + c + '\'.')

def eval(line):
    stream = PeekableStream(line)

    while stream.next is not None:
        tok = stream.move_next()

        if tok[0] == 'register':
            yield ('0x0000', str(ord(tok[1].lower()) - 97))
        elif tok[0] == 'address':
            yield ('0x0001', lex_line(tok[1]))
        elif tok[0] == 'ptr':
            if tok[1][0] == '%':
                yield ('0x0003', str(ord(tok[1][1].lower()) - 97))
        elif tok[0] == 'hex':
            if ():
                pass
            yield ('0x0002', str(tok[1]))
        elif tok[0] == 'dec':
            yield ('0x0002', tok[1])
        elif tok[0] == 'instruct':
            yield instructs[tok[1].lower()]
        else:
            yield tok

arg_parser = argparse.ArgumentParser(description='Assembles a Pog Assembly Program')

arg_parser.add_argument(
    'file', type=str, help='File to assemble'
)
arg_parser.add_argument(
    '-o', '--out', type=str, default='a.bin', help='output file, a.bin by default'
)

args = arg_parser.parse_args()

f = open(args.out, 'wb')

for line in open(args.file, 'r').readlines():
    byte_line = []
    for i in eval(lex(line)):
        if type(i) is str:
            byte_line.append(i)
        elif type(i) is tuple:
            byte_line += i
    for i, val in enumerate(byte_line):
        try:
            byte_line[i] = int(val, 16).to_bytes(2, 'big')
        except ValueError:
            raise SyntaxError('Invalid Hex Bytes') from None
    byte_line = pad(byte_line, b'\xff\xff', 5)

    f.write(bytearray(b''.join(i for i in byte_line)))
