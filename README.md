D2MCE
===========
## Description
Dynamic Distributed Mobile Computing Environment (D2MCE) designed for
mobile device environment. It’s can dynamic join and leave the distributed computing
architecture and use the distribute share memory to share the availability resources. In
this framework, communication between a mobile devices access the share memory like
the multi-threaded program. We can easily write a network program or a distributed
network parallel computing program. D2MCE is easier than use message passing (ex.
socket) or Message Passing Interface (MPI) to transmit information.

There will fouce on the D2MCE’s performance improvement and optimization in
the thesis. Include 

1. New D2MCE architecture can grain the more performance in the
Multi-core CPU plateform. 
2. Library is thread safe, developers can user D2MCE to
write the multi-thread program. 
3. Multi-Writer protocol, solve the false sharing issues
and provide developers a better information sharing model。
4. Manager migration,
resources can be moved to a different node do the load balance. 
5. Disseminate update
protocol, designed for a small amount of data sharing for multi nodes will grain better
performance and reduce the number of communication. 
6. Event driven, a new novel
acquire who is more adaptation for network applications.

## Document
[PDF](https://drive.google.com/file/d/0B0kAz7usd295NTIxMWJjZTctYmQzNy00MDBjLTlhZGMtMWJiM2VjZDYwZDZl)

[PPT](http://www.slideshare.net/ZongYingLyu/ss-37804388)


## Make library & demon
Only in 32bit linux. ex: ubuntu 14.04 32bit
```javascript
cp conf/d2mce.conf ~/.d2mce_conf/
cd src/
make
```

## Run demon

```javascript
./d2mced
```

## Example
```javascript
cd examples/app
make
./app

```
