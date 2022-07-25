#!/usr/bin/env python
# encoding: utf-8
# Revisión 2019 (a Python 3 y base64): Pablo Ventura
# Revisión 2014 Carlos Bederián
# Revisión 2011 Nicolás Wolovick
# Copyright 2008-2010 Natalia Bidart y Daniel Moisset
# $Id: server.py 656 2013-03-18 23:49:11Z bc $

import optparse
import os
import socket
import sys

import connection
from constants import *
import multiprocessing as mp


def handle_conn(client_con):
    # while True:
    #     recv_data = client_con.s.recv(4096)
    #     if recv_data:
    #         if client_con.handle(recv_data):
    #             break
    #     else:
    #         break

    while True:
        recv_data = client_con.s.recv(1024)
        if recv_data:
            if client_con.handle(recv_data):
                break
        else:
            break


class Server(object):
    """
    El servidor, que crea y atiende el socket en la dirección y puerto
    especificados donde se reciben nuevas conexiones de clientes.
    """

    def __init__(self, addr=DEFAULT_ADDR, port=DEFAULT_PORT,
                 directory=DEFAULT_DIR):
        print("Serving %s on %s:%s." % (directory, addr, port))
        self.addr = addr
        self.port = port
        self.directory = directory
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.bind((addr, port))
        self.s.listen()
        mp.set_start_method('spawn')
        self.process_modules = mp.Pool(os.cpu_count())
        print(f'CPU cores -> {os.cpu_count()} \n')

    def serve(self):
        """
        Loop principal del servidor. Se acepta una conexión a la vez
        y se espera a que concluya antes de seguir.
        """
        while True:
            try:
                client_sock, client_addr = self.s.accept()
                client_con = connection.Connection(client_sock, self.directory)
                self.process_modules.apply_async(func=handle_conn, args=(client_con,))
            except Exception as e:
                print(e)


def main():
    """Parsea los argumentos y lanza el server"""

    parser = optparse.OptionParser()
    parser.add_option(
        "-p", "--port",
        help="Número de puerto TCP donde escuchar", default=DEFAULT_PORT)
    parser.add_option(
        "-a", "--address",
        help="Dirección donde escuchar", default=DEFAULT_ADDR)
    parser.add_option(
        "-d", "--datadir",
        help="Directorio compartido", default=DEFAULT_DIR)

    options, args = parser.parse_args()
    if len(args) > 0:
        parser.print_help()
        sys.exit(1)
    try:
        port = int(options.port)
    except ValueError:
        sys.stderr.write(
            "Numero de puerto invalido: %s\n" % repr(options.port))
        parser.print_help()
        sys.exit(1)

    server = Server(options.address, port, options.datadir)
    server.serve()


if __name__ == '__main__':
    main()
