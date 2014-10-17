#!/usr/bin/env python
# coding: utf-8

import sinastorage

project = 'sandbox'
acckey = 'SYS0000000000SANDBOX'
seckey = '1' * 40

cli = sinastorage.S3( acckey, seckey, project )
cli.need_auth = True

cli.upload_file( 'test', 'example.py' )
rst = cli.get_file( 'test' )
print rst
