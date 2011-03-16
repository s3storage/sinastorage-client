
import datetime
import types
import hashlib
import hmac

def uploadquery( nation, accesskey, secretkey,
                 project, key,
                 Length, hashinfo,
                 expires = None,
                 metas = {},
                 relax = False,
                 **kwargs ) :
    
    
    hl = len(hashinfo)
    
    if hl == 40 : #sha1 hex
        hk = 's-sina-sha1'
    elif hl == 28 :
        hk = 'Content-SHA1'
    elif hl == 32 :
        hk = 's-sina-md5'
    elif hl == 24 :
        hk = 'Content-MD5'
    
    et = type(expires)
    
    if et in types.StringTypes :
        dt = expires.encode('utf-8')
    elif et == NoneType :
        dt = datetime.datetime.utcnow()+datetime.timedelta( seconds=900 )
        dt = dt.strftime('%a, %d %b %Y %H:%M:%S +0000')
    elif et == datetime.timedelta :
        dt = datetime.datetime.utcnow()+QueryExpires
        dt = dt.strftime('%a, %d %b %Y %H:%M:%S +0000')
    elif et == datetime.datetime :
        dt = dt.strftime('%a, %d %b %Y %H:%M:%S +0000')
    
    metas = metas.copy()
    
    metas[hk] = hashinfo
    
    if 'decovered' in kwargs :
        metas['x-sina-decovered'] = bool(kwargs['decovered'])
    
    if 'autoclean' in kwargs :
        ac = kwargs['autoclean']
        tac = type(ac)
        if tac in ( types.IntType, types.LongType ) :
            metas['s-sina-expires-ctime'] = "%d" % (ac,)
        elif tac in types.StringTypes :
            metas['s-sina-expires'] = ac.encode('utf-8')
        elif tac == datetime.datetime :
            metas['s-sina-expires'] = ac.strftime('%a, %d %b %Y %H:%M:%S +0000')
    
    key = key.encode('utf-8')
    resource = "/"+str(project)+"/"+key
    
    h = kwargs.get('vhost', False )
    url = '/'+key if h else resource
    
    qs = []
    
    if relax :
        resource += '?relax'
        qs += ['relax']
        metas['s-sina-length'] = str(Length)
    else :
        metas['Content-Length'] = str(Length)
    
    ct = metas.get('Content-Type', '')
    
    print metas
    
    mts = [ (str(k).lower(), v.encode('utf-8')) for k, v in metas.items() ]
    mts = [ k+':'+v for k, v in mts if k.startswith('x-sina-') or k.startswith('x-amz-meta-') ]
    mts.sort()
    
    stringtosign = '\n'.join( ["PUT", hashinfo, ct, dt] + mts + [resource] )
    
    ssig = hmac.new( secretkey, stringtosign, hashlib.sha1 ).digest().encode('base64')
    
    metas['Date'] = dt
    metas['Authorization'] = nation.upper() + ' ' + accesskey + ':' + \
                             ssig[5:15]
    
    url += '&'.join(qs)
    
    return url, metas
    
    
if __name__ == '__main__':
    
    print uploadquery( 'sina', 'product', 
                       'uV3F3YluFJax1cknvbcGwgjvx4QpvB+leU8dUj2o',
                       'yourproject', 'abc/def.jpg', 
                       11, 'XrY7u+Ae7tCTyyK7j1rNww==', 
                       expires='Tue, 10 Aug 2010 16:08:08 +0000',
                       metas = { 'Content-Type': 'text/plain',
                                 'Content-Encoding': 'utf-8',
                                 'X-Sina-Info': '%E4%B8%AD%E6%96%87',
                               }
                       )