#!/usr/bin/env python2.6
# coding: utf-8


if __name__ == "__main__":

    import sinastorage

    resp = sinastorage.put( "218.30.126.203", 80,
                            "sys", "sandbox", "1" * 40,
                            "sandbox2", "testfile", "sinastorage.py"
    )

    print resp.status == 200

    resp = sinastorage.get( "218.30.126.203", 80,
                            "sys", "sandbox", "1" * 40,
                            "sandbox2", "testfile"
    )

    print resp.status == 200
    print resp.read()
