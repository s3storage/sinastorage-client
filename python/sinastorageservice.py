
import os
import sys
import stat
import copy
import time
import types
import datetime

# # compatible with python2.4
# import hashlib
try:
    import hashlib
    sha1 = hashlib.sha1
except ImportError:
    import sha
    class Sha1:
        digest_size = 20
        def new( self, inp = '' ):
            return sha.sha( inp )

    sha1 = Sha1()

import hmac
import urllib
import httplib
import mimetypes


def ftype( f ):
    tp = mimetypes.guess_type( f )[ 0 ]
    return tp if tp is not None else ''


def fsize( f ):
    return os.path.getsize( f )



class S3Error( Exception ): pass


class S3( object ):

    """ python SDK for Sina Storage Service """

    DEFAULT_DOMAIN = 'sinastorage.com'
    DEFAULT_UP_DOMAIN = 'up.sinastorage.com'

    HTTP_OK = 200
    HTTP_DELETE = 204

    CHUNK = 1024 * 1024

    QUERY_STRINGS = [ 'ip', 'foo' ]

    def __init__( self, accesskey = None, secretkey = None, project = None ):

        self.accesskey = 'SYS0000000000SANDBOX' if accesskey is None else accesskey

        if len( self.accesskey ) != len( 'SYS0000000000SANDBOX' ) \
                or '0' not in self.accesskey:
            raise S3Error, 'accesskey "%s" is illegal.' % self.accesskey

        #UNDO like '0000000000xxxxxxxxxx'
        self.nation = self.accesskey.split( '0' )[0].lower()
        if self.nation == 'sae':
            self.accesskey = self.accesskey[ -10: ].lower()
        else:
            self.accesskey = self.accesskey.split( '0' )[-1].lower()

        self.secretkey = '1' * 40 if secretkey is None else secretkey
        self.project = 'sandbox' if project is None else project

        self.purge()


    def purge( self ):

        self.domain = self.DEFAULT_DOMAIN
        self.up_domain = self.DEFAULT_UP_DOMAIN

        self.port = 80
        self.timeout = 60

        self.expires = time.time().__int__() + 30 * 60

        self.extra = '?'
        self.query_string = {}
        self.requst_header = {}

        self.is_ssl = False
        self.ssl_auth = {}

        self.need_auth = False

        self.vhost = False


    def set_https( self, **ssl ):
        self.is_ssl = True
        self.port = 4443
        self.timeout = 3 * 60

        self.ssl_auth['key_file'] = ssl.get( 'key_file', '')
        self.ssl_auth['cert_file'] = ssl.get( 'cert_file', '')

    def set_domain( self, domain ):
        self.domain = domain

    def set_port( self, port ):
        self.port = int( port )

    def set_timeout( self, timeout ):
        self.timeout = int( timeout )

    def set_expires( self, expires ):
        self.expires = expires

    def set_expires_delta( self, delta ):
        self.expires = time.time().__init__() + int( delta )

    def set_extra( self, extra ):
        self.extra = extra

    def set_need_auth( self, auth = True ):
        self.need_auth = auth

    def set_vhost( self, vhost = True ):

        if vhost:
            self.domain = str( self.project )
        else:
            self.domain = self.DEFAULT_DOMAIN

        self.vhost = bool( vhost )

    def set_query_string( self, **kwargs ):

        for k, v in kwargs.items():
            self.query_string[ k ] = v

    def set_requst_header( self, **kwargs ):

        for k, v in kwargs.items():
            self.requst_header[ k ] = v


    # large file upload steps:
    # 1. get upload idc : get a domain to hold during uploading a file
    # 2. get upload id  : get a uploadid to bind during uploading parts
    # 3. upload part    : upload a part
    # 4. list parts     : list the parts that are uploaded to server
    # 5. merge part     : merge all parts after uplaod all parts

    def get_upload_idc( self ):

        self.set_domain( self.up_domain )

        uri = '/?extra&op=domain.json'

        rh = self.requst_header

        try:
            h = self._http_handle()
            h.putrequest( 'GET', uri )
            for k in rh:
                h.putheader( k, rh[ k ] )
            h.endheaders()
            resp = h.getresponse()

            if resp.status == self.HTTP_OK:
                return True, resp.read().strip().strip( '"' )
            else:
                return False, resp

        except Exception, e:
            raise S3Error, " Get upload idc error : '%s' " % \
                    ( repr( e ), )

    def get_upload_id( self, key, ct = None ):

        if self.domain == self.DEFAULT_DOMAIN:
            self.set_domain( self.up_domain )

        rh = { 'Content-Type' : str( ct ) } if \
                ct is not None else {}

        uri = self._signature( 'POST', key, \
                                ex_extra = 'uploads',
                                ex_requst_header = rh,
                                )

        rh.update( self.requst_header )

        try:
            h = self._http_handle()
            h.putrequest( 'POST', uri )
            for k in rh:
                h.putheader( k, rh[ k ] )
            h.endheaders()
            resp = h.getresponse()

            if resp.status != self.HTTP_OK:
                return False, resp

            data = resp.read()

            import re

            r = re.compile( '<UploadId>(.{32})</UploadId>' )
            r = r.search( data )

            if r:
                return True, r.groups()[0]
            else:
                raise S3Error, " '%s' get uploadid failed. '%s'" % \
                        ( key, data, )

        except Exception, e:
            raise S3Error, " '%s' get uploadid error : '%s'" % \
                            ( key, repr( e ), )

    def upload_part( self, key, uploadid, partnum, partfile, ct = None, cl = None ):

        if self.domain == self.DEFAULT_DOMAIN:
            self.set_domain( self.up_domain )

        qs = {  'uploadId' : str( uploadid ),
                'partNumber' : str( partnum ) }

        rh = {  'Content-Type' : str( ct ) if ct is not None \
                                        else str( ftype( partfile ) ),
                'Content-Length' : str( cl ) if cl is not None \
                                        else str( fsize( partfile ) ) }

        uri = self._signature( 'PUT', key,
                                ex_query_string = qs,
                                ex_requst_header = rh,
                                )

        rh.update( self.requst_header )

        f = open( partfile, 'rb' )
        try:
            h = self._http_handle()
            h.putrequest( 'PUT', uri )
            for k in rh:
                h.putheader( k, rh[ k ] )
            h.endheaders()

            while True:
                data = f.read( self.CHUNK )
                if data == '':
                    break
                h.send( data )

            resp = h.getresponse()

            return resp.status == self.HTTP_OK, resp

        except Exception, e:
            raise S3Error, " '%s' upload part '%s:%s', error : '%s'" % \
                            ( key, uploadid, str( partnum ), repr( e ), )

        finally:
            f.close()

    def list_parts( self, key, uploadid ):

        if self.domain == self.DEFAULT_DOMAIN:
            self.set_domain( self.up_domain )

        qs = { 'uploadId' : str( uploadid ) }

        uri = self._signature( 'GET', key,
                                ex_query_string = qs,
                                )

        rh = self.requst_header

        try:
            h = self._http_handle()
            h.putrequest( 'GET', uri )
            for k in rh:
                h.putheader( k, rh[ k ] )
            h.endheaders()

            resp = h.getresponse()

            if resp.status != self.HTTP_OK:
                return False, resp

            data = resp.read().strip()

            import re

            tr = re.compile( '<IsTruncated>(True|False)</IsTruncated>' )
            tr = tr.search( data )

            if tr and tr.groups()[0] == 'True':
                tr = True
            else:
                tr = False

            pr = re.compile( '<PartNumber>([0-9]*)</PartNumber>' )
            pr = pr.findall( data )

            if pr:
                pr = [ int( i ) for i in pr ]
                pr.sort()
            else:
                tr = True
                pr = []

            return True, pr
            #return True, ( tr, pr[ : ] )

        except Exception, e:
            raise S3Error, " '%s' list parts, uploadid '%s', error : '%s'" % \
                            ( key, uploadid, repr( e ), )

    def merge_parts( self, key, uploadid, mergefile, ct = None, cl = None ):

        if self.domain == self.DEFAULT_DOMAIN:
            self.set_domain( self.up_domain )

        qs = {  'uploadId' : str( uploadid ) }

        rh = {  'Content-Type' : str( ct ) if ct is not None \
                                        else str( ftype( mergefile) ),
                'Content-Length' : str( cl ) if cl is not None \
                                        else str( fsize( mergefile ) ) }

        uri = self._signature( 'POST', key,
                                ex_query_string = qs,
                                ex_requst_header = rh,
                                )

        rh.update( self.requst_header )

        f = open( mergefile, 'rb' )
        try:
            h = self._http_handle()
            h.putrequest( 'POST', uri )
            for k in rh:
                h.putheader( k, rh[ k ] )
            h.endheaders()

            while True:
                data = f.read( self.CHUNK )
                if data == '':
                    break
                h.send( data )

            resp = h.getresponse()

            return resp.status == self.HTTP_OK, resp

        except Exception, e:
            raise S3Error, " '%s' merge file, uplodid '%s', error : '%s'" % \
                            ( key, uploadid, repr( e ), )

        finally:
            f.close()



    def upload_file( self, key, fn ):

        rh = {  'Content-Type' : str( ftype( fn ) ),
                'Content-Length' : str( fsize( fn ) ) }

        uri = self._signature( 'PUT', key )

        rh.update( self.requst_header )

        f = open( fn, 'rb' )
        try:
            h = self._http_handle()
            h.putrequest( 'PUT', uri )
            for k in rh:
                h.putheader( k, rh[ k ] )
            h.endheaders()

            while True:
                data = f.read( self.CHUNK )
                if data == '':
                    break
                h.send( data )

            resp = h.getresponse()

            return resp.status == self.HTTP_OK, resp

        except Exception, e:
            raise S3Error, " '%s' upload file error : '%s'" % \
                            ( key, repr( e ), )

        finally:
            f.close()


    def download( self, key ):

        return self.getFile( key )


    def getFile( self, key ):

        args = self.downloadquery( key )

        uri = args[ 0 ]

        try:
            h = self._http_handle()
            h.putrequest( 'GET', uri )
            h.endheaders()

            resp = h.getresponse()

            if resp.status == self.HTTP_OK:
                return True, resp.read()
            else:
                return False, resp

        except Exception, e:
            raise S3Error, "getfile '%s' error : '%s'" % \
                        ( key, repr( e ), )

    def getFileUrl( self, key ):

        args = self.downloadquery( key )

        uri = args[ 0 ]

        return True, 'http://%s%s' % \
                    ( self.DEFAULT_DOMAIN, uri )

    def _put( self, key, fn ):

        flen = os.path.getsize( fn )

        #args = self.uploadquery( 'PUT', key )

        uri = self._signature( 'PUT', key )
        print uri

        f = open( fn, 'rb' )
        try:
            h = self._http_handle()
            h.putrequest( 'PUT', uri )
            h.putheader( "Content-Length", str( flen ) )
            h.endheaders()

            while True:
                data = f.read( 1024 * 1024 )
                if data == '':
                    break
                h.send( data )
            resp = h.getresponse()
            return resp.status == self.HTTP_OK, resp

        finally:
            f.close()


    def relax_upload( self, rsha1, rlen ):

        self.extra = '?relax'

        metas = {}
        metas['s-sina-sha1'] = str( rsha1 )
        metas['s-sina-length'] = str( rlen )


    def _http_handle( self ):

        try:
            if self.is_ssl:
                h = httplib.HTTPSConnection(    self.domain, \
                                                self.port, \
                                                timeout = self.timeout, \
                                                **self.ssl_auth )
            else:
                h = httplib.HTTPConnection(     self.domain, \
                                                self.port, \
                                                timeout = self.timeout, )
        except httplib.HTTPException, e:

            raise S3Error, "Connect '%s:%s' error : '%s' " % \
                    ( self.domain, self.port, repr( e ), )

        return h


    def _signature( self, verb, key, \
                    ex_extra = '', \
                    ex_query_string = None, \
                    ex_requst_header = None ):

        extra = self.extra + ex_extra

        query_string = copy.deepcopy( self.query_string )
        query_string.update( ex_query_string if \
                            ex_query_string is not None else {}  )

        requst_header = copy.deepcopy( self.requst_header )
        requst_header.update( ex_requst_header if \
                            ex_requst_header is not None else {} )

        key = key.encode( 'utf-8' )
        uri = ''

        if self.vhost:
            uri = '/' + key
        else:
            uri = "/" + str( self.project ) + "/" + key
        if extra != '?':
            uri += extra + '&'
        else:
            uri += extra

        qs = '&'.join( [ '%s=%s' % ( k, v, ) for \
                            k, v in query_string.items() ] )
        uri += qs + '&' if qs != '' else ''

        if not self.need_auth:
            return uri.rstrip( '?&' )

        rh = dict( [ ( str( k ).lower(), v.encode( 'utf-8' ) ) for \
                k, v in requst_header.items() ] )

        for t in ( 's-sina-sha1', 'content-sha1', \
                's-sina-md5', 'content-md5' ):
            if t in rh:
                rh[ 'hash-info' ] = rh[ t ]
                break

        verb = verb.upper()
        hashinfo = rh.get( 'hash-info', '' )
        ct = rh.get( 'content-type', '' )

        et = type( self.expires )
        if et in ( types.IntType, types.LongType, types.FloatType ):
            dt = str( int( self.expires ) )
        elif et in types.StringTypes :
            dt = self.expires.encode( 'utf-8' )
        elif et == types.NoneType :
            dt = datetime.datetime.utcnow()
            dt = dt.strftime( '%a, %d %b %Y %H:%M:%S +0000' )
        elif et == datetime.timedelta :
            dt = datetime.datetime.utcnow() + self.expires
            dt = dt.strftime( '%a, %d %b %Y %H:%M:%S +0000' )
        elif et == datetime.datetime :
            dt = dt.strftime( '%a, %d %b %Y %H:%M:%S +0000' )
        else:
            dt = time.time().__int__() + 30 * 60

        mts = [ k + ':' + v for k, v in rh.items() \
                if k.startswith( 'x-sina-' ) or \
                    k.startswith( 'x-amz-' ) ]
        mts.sort()

        stringtosign = '\n'.join( [ verb, hashinfo, ct, dt ] + mts + [ uri.rstrip( '?&' ) ] )
        #print stringtosign

        ssig = hmac.new( self.secretkey, stringtosign, sha1 ).digest().encode( 'base64' )

        uri += "&".join( [  "KID=" + self.nation.lower() + "," + self.accesskey,
                            "Expires=" + dt,
                            "ssig=" + urllib.quote_plus( ssig[5:15] ), ] )

        return uri.rstrip( '?&' )




if __name__ == '__main__':

    pass

