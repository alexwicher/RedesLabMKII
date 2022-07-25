# encoding: utf-8
# Revisión 2019 (a Python 3 y base64): Pablo Ventura
# Copyright 2014 Carlos Bederián
# $Id: connection.py 455 2011-05-01 00:32:09Z carlos $
import base64
import math
import os
from pathlib import Path

from constants import *


def isInt(number):
    try:
        int(number)
        return True
    except ValueError:
        return False


class Connection(object):
    """
    Conexión punto a punto entre el servidor y un cliente.
    Se encarga de satisfacer los pedidos del cliente hasta
    que termina la conexión.
    """

    def __init__(self, socket, directory):
        self.s = socket
        self.dir = directory
        self.cmd = ''
        self.line = None

    def handle(self, recv_data):
        try:
            if recv_data:
                # print('--> ' + str(recv_data))
                eol_lines = recv_data.decode("ascii").split(EOL)

                if self.line:  # Concateno linea sin EOL del pqt previo a la del pqt de ahora
                    eol_lines[0] = self.line + eol_lines[0]
                    self.line = None

                if eol_lines[eol_lines.__len__() - 1] != "":  # Split da "" si hay EOL al final
                    self.line = eol_lines.pop(eol_lines.__len__() - 1)  # Puede escapar una comando mirar (*) en line 54
                else:
                    eol_lines.pop(eol_lines.__len__() - 1)

                if self.line and EOL in self.line:  # (*) para caso raro que llega tipo {algo}\r y luego \n{algo}\r
                    aux = self.line
                    self.line = None
                    self.handle(aux.encode("ascii"))

                for line in eol_lines:
                    if self.badEOL(line):  # Chequeo por bad EOL
                        self.s.send(b'100 FAIL: bad EOL\r\n')
                        return
                    if not self.validateChars(line):
                        return
                    fullCmd = line.split(' ')
                    cmd = fullCmd[0]
                    args = fullCmd[1:fullCmd.__len__()]
                    if cmd == 'get_file_listing':
                        self.file_listing(args)
                    elif cmd == 'get_metadata':
                        self.metadata(args)
                    elif cmd == 'quit':
                        return self.quit(args)
                    elif cmd == 'get_slice':
                        self.slice(args)
                    else:
                        self.s.send(b'200 FAIL: command not recognized\r\n')

                # if self.line and len(self.line) >= MAX_LENGHT_CMD:
                #     self.line += EOL
            return KEEP_LISTENING
        except Exception as e:
            print(e)
            self.s.close()

    # Aux ----------------------------------------------------------

    def badEOL(self, str):
        for ind, ch in enumerate(str[:-1]):
            if ch == '\n':
                if ind == 0 or str[ind - 1] != '\r':
                    return True
        return False

    def validateChars(self, str):
        data = str.replace(EOL, ' ')
        res = (False not in [a in VALID_CHARS for a in data])
        if not res:
            self.s.send(b'101 FAIL No danger chars, stop!\r\n')
        return res

    # Commands ----------------------------------------------------------

    def quit(self, args):
        print('-- quit --')
        if args.__len__() != 0:
            self.s.send(b'201 FAIL: cmd requires zero inputs\r\n')
            return KEEP_LISTENING
        self.s.send(b'0 OK\r\n')
        self.s.close()
        return STOP_LISTENING

    def metadata(self, args):
        print('-- metadata --')
        if args.__len__() != 1:
            self.s.send(b'201 FAIL: cmd requires one input -> filename\r\n')
            return
        if not args[0] in os.listdir(self.dir):
            self.s.send(b'202 FAIL: file not found\r\n')
            return
        sizeOfFile = Path(self.dir + "/" + args[0]).stat().st_size
        self.s.send(b'0 OK\r\n' + (str(sizeOfFile) + EOL).encode("ascii"))

    def file_listing(self, args):
        print('-- fileListing --')
        if args.__len__() != 0:
            self.s.send(b'201 FAIL: cmd requires zero inputs\r\n')
            return
        listDir = EOL.join(os.listdir(self.dir))
        self.s.send(b'0 OK\r\n ' + (listDir + EOL + EOL).encode("ascii"))

    def slice(self, args):
        print('-- slice --')
        if args.__len__() != 3:
            self.s.send(b'201 FAIL: cmd requires three inputs -> filename offset size\r\n')
            return
        if not isInt(args[1]) or not isInt(args[2]):
            self.s.send(b'201 FAIL: off and size must be int\r\n')
            return
        file = args[0]
        off = int(args[1])
        length = int(args[2])
        if off < 0 or length < 0:
            self.s.send(b'201 FAIL: no negative numbers\r\n')
            return
        if not file in os.listdir(self.dir):
            self.s.send(b'202 FAIL: file not found\r\n')
            return
        fileSize = Path(self.dir + "/" + file).stat().st_size
        if fileSize == 0:
            self.s.send(b'201 FAIL: file size is 0\r\n')
        if length + off > fileSize:
            self.s.send(
                b'201 FAIL: offset + length too large\r\n')
            return

        data = open(self.dir + "/" + file, "rb").read()
        data = data[off:off + length]

        # https://stackoverflow.com/questions/34109053/what-file-size-is-data-if-its-450kb-base64-encoded

        frag_size = 2048  # 2700 # aprox
        l = math.ceil(data.__len__() / frag_size)
        self.s.send(b'0 OK\r\n')
        for i in range(0, l):
            if i == l - 1:
                frag = data[i * frag_size:data.__len__()]
            else:
                frag = data[i * frag_size:(i + 1) * frag_size]
            frag = base64.b64encode(frag)
            self.s.sendall(b'' + frag + EOL.encode('ascii'))
