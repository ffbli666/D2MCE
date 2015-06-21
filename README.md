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
