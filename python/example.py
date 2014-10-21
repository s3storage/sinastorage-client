#!/usr/bin/env python
# coding: utf-8

import urllib2
import sinastorage

project = 'sandbox2'
acckey = 'SYS0000000000SANDBOX'
seckey = '1' * 40

cli = sinastorage.S3( acckey, seckey, project )
cli.need_auth = True

cli.upload_file( 'test', 'example.py' )
rst = cli.get_file( 'test' )
print rst


# serverside key
rst = cli.upload_data( 'ssk/', 'foo' )
print rst[ 'ssk' ]

k = rst[ 'ssk' ]
k = urllib2.unquote( k )
print k

rst = cli.get_file( k )
print rst
