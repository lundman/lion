# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _lion

def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name) or (name == "thisown"):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types



lion_userinput = _lion.lion_userinput
LION_PIPE_FAILED = _lion.LION_PIPE_FAILED
LION_PIPE_RUNNING = _lion.LION_PIPE_RUNNING
LION_PIPE_EXIT = _lion.LION_PIPE_EXIT
LION_FILE_OPEN = _lion.LION_FILE_OPEN
LION_FILE_CLOSED = _lion.LION_FILE_CLOSED
LION_FILE_FAILED = _lion.LION_FILE_FAILED
LION_CONNECTION_CLOSED = _lion.LION_CONNECTION_CLOSED
LION_CONNECTION_CONNECTED = _lion.LION_CONNECTION_CONNECTED
LION_CONNECTION_NEW = _lion.LION_CONNECTION_NEW
LION_CONNECTION_LOST = _lion.LION_CONNECTION_LOST
LION_BUFFER_EMPTY = _lion.LION_BUFFER_EMPTY
LION_BUFFER_USED = _lion.LION_BUFFER_USED
LION_BINARY = _lion.LION_BINARY
LION_INPUT = _lion.LION_INPUT
LION_CONNECTION_SECURE_FAILED = _lion.LION_CONNECTION_SECURE_FAILED
LION_CONNECTION_SECURE_ENABLED = _lion.LION_CONNECTION_SECURE_ENABLED
LION_TYPE_NONE = _lion.LION_TYPE_NONE
LION_TYPE_SOCKET = _lion.LION_TYPE_SOCKET
LION_TYPE_FILE = _lion.LION_TYPE_FILE
LION_TYPE_PIPE = _lion.LION_TYPE_PIPE
LION_TYPE_UDP = _lion.LION_TYPE_UDP
LION_FLAG_NONE = _lion.LION_FLAG_NONE
LION_FLAG_FULFILL = _lion.LION_FLAG_FULFILL
LION_FLAG_EXCLUSIVE = _lion.LION_FLAG_EXCLUSIVE

lion_init = _lion.lion_init

lion_free = _lion.lion_free

lion_compress_level = _lion.lion_compress_level

lion_buffersize = _lion.lion_buffersize

lion_poll = _lion.lion_poll

lion_find = _lion.lion_find

lion_setbinary = _lion.lion_setbinary

lion_close = _lion.lion_close

lion_disconnect = _lion.lion_disconnect

lion_printf = _lion.lion_printf

lion_send = _lion.lion_send

lion_output = _lion.lion_output

lion_disable_read = _lion.lion_disable_read

lion_enable_read = _lion.lion_enable_read

lion_set_userdata = _lion.lion_set_userdata

lion_get_userdata = _lion.lion_get_userdata

lion_gettype = _lion.lion_gettype

lion_fileno = _lion.lion_fileno

lion_get_bytes = _lion.lion_get_bytes

lion_get_duration = _lion.lion_get_duration

lion_get_cps = _lion.lion_get_cps

lion_rate_in = _lion.lion_rate_in

lion_rate_out = _lion.lion_rate_out

lion_addr = _lion.lion_addr

lion_isconnected = _lion.lion_isconnected

lion_connect = _lion.lion_connect

lion_listen = _lion.lion_listen

lion_accept = _lion.lion_accept

lion_getsockname = _lion.lion_getsockname

lion_getpeername = _lion.lion_getpeername

lion_ftp_port = _lion.lion_ftp_port

lion_ftp_pasv = _lion.lion_ftp_pasv

lion_ntoa = _lion.lion_ntoa
LION_SSL_OFF = _lion.LION_SSL_OFF
LION_SSL_CLIENT = _lion.LION_SSL_CLIENT
LION_SSL_SERVER = _lion.LION_SSL_SERVER
LION_SSL_FILE = _lion.LION_SSL_FILE

lion_ssl_set = _lion.lion_ssl_set

lion_ssl_enabled = _lion.lion_ssl_enabled

lion_ssl_ciphers = _lion.lion_ssl_ciphers

lion_ssl_rsafile = _lion.lion_ssl_rsafile

lion_ssl_egdfile = _lion.lion_ssl_egdfile

lion_ssl_setkey = _lion.lion_ssl_setkey

lion_ssl_clearkey = _lion.lion_ssl_clearkey

lion_open = _lion.lion_open

lion_fork = _lion.lion_fork

lion_execve = _lion.lion_execve

lion_system = _lion.lion_system

lion_want_returncode = _lion.lion_want_returncode

lion_exitchild = _lion.lion_exitchild

lion_get_handler = _lion.lion_get_handler

lion_group_new = _lion.lion_group_new

lion_group_free = _lion.lion_group_free

lion_group_add = _lion.lion_group_add

lion_group_remove = _lion.lion_group_remove

lion_group_rate_in = _lion.lion_group_rate_in

lion_group_rate_out = _lion.lion_group_rate_out

lion_global_rate_in = _lion.lion_global_rate_in

lion_global_rate_out = _lion.lion_global_rate_out

lion_udp_new = _lion.lion_udp_new

lion_udp_bind = _lion.lion_udp_bind

lion_udp_bind_handle = _lion.lion_udp_bind_handle

lion_add_multicast = _lion.lion_add_multicast

lion_drop_multicast = _lion.lion_drop_multicast
cvar = _lion.cvar

lion_set_handler = _lion.lion_set_handler

