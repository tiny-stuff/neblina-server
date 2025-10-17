## Tasks

- [x] Rename project
- [x] Orchestrator
- [x] TCP server + Line by Line + Parrot
- [x] Multithreaded connection pool
- [x] Improve OS independence
  - [x] Migrate to CMake
  - [x] Improve OS separation of concerns (posix by default)
  - [x] Tests
- [x] Code review
  - [x] Fix all warnings
  - [x] Move CommunicationBuffer inside Session (as ConnectionBuffer)
  - [x] Document stuff
  - [x] Proper error handling
  - [x] Github actions to build/test in all envs
  - [x] Create class creator
  - [x] Additional tests (for parrot)
    - [x] Create TCPClient (?)
    - [x] Load testing
- [ ] General Improvements
  - [x] Doctest
  - [x] Futures
  - [ ] String library?
- [ ] SSL server + SParrot
  - [ ] Handle disconnect (free SSL)
  - [ ] SSL client
- [x] SSL server + SParrot
  - [x] Leaks
  - [x] Handle disconnect (free SSL)
  - [ ] Write tests
  - [ ] Refactor
    - [ ] Use void* for polymorhism
    - [ ] Consider composition over inheritance
- [ ] Process config file
- [ ] HTTP structs
- [ ] HTTP server and request handlers
  - [ ] Default 404
  - [ ] Health
  - [ ] Static website
  - [ ] Redirect
  - [ ] Proxy
- [ ] Rewrite queue handling in Windows using IOCP instead of WSApoll
- [ ] HTTPS
- [ ] HTTP-to-HTTPS
- [ ] FTP
- [ ] SFTP
- [ ] XMMP
- [ ] NNTP
- [ ] External services
  - [ ] Gopher

## External application

- [ ] Database access
- [ ] CORS
- [ ] Example: guestbook (as external service)

## Auth

- [ ] Basic auth
- [ ] Oauth2
- [ ] Web tool to manage users

## Web applictions

- [ ] Config tool (first execution)
- [ ] Markdown webpage
- [ ] Password manager
- [ ] Note taking
- [ ] Notepad
- [ ] Blog
- [ ] Social network
- [ ] Photo/video app
- [ ] Podcast

## Include in HTTP

- [ ] Require Host request (otherwise 400)
- [ ] Automatic error according to request type
- [ ] Persistent connections (check CommunicationBuffer header)
- [ ] Transfer encoding: gzip, deflate
- [ ] Redirects
- [ ] Conditional requests: If-None-Match, ETag
- [ ] Run connection/ session in threads
- [ ] Transfer-Encoding (chunks)
- [ ] Streaming
- [ ] Caching: If-Modified-Since, Last-Modified
- [ ] Support everything in the RFCs
- [ ] Control the time that the connection is open, close it after a while
- [ ] Dock app on Windows, Linux
