#sinastorage-client
```graph
    A[Hard edge] -->|Link text| B(Round edge)
    B --> C{Decision}
    C -->|One| D[Result one]
    C -->|Two| E[Result two]
    C --> F
    F --> A
    A --> G
    
    ip(intra-ip) --> NAT
    nb(NingBo-mysql) --> ip(intra-ip)
    yc(Yachuan) --> nb
    nb --> bk(mysql-backup)
    bili(bilibili) --> nb
    
```