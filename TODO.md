## Tasks

- [x] Rename project
- [x] Orchestrator
- [x] TCP server + Line by Line + Parrot
- [x] Multithreaded connection pool
- [x] Improve OS independence
  - [x] Migrate to CMake
  - [x] Improve OS separation of concerns (posix by default)
  - [x] Tests
- [ ] Code review
  - [x] Fix all warnings
  - [x] Move CommunicationBuffer inside Session (as ConnectionBuffer)
  - [x] Document stuff
  - [ ] Create class creator
  - [ ] Proper error handling
  - [ ] Additional tests (for parrot)
  - [ ] Github actions to build/test in all envs
- [ ] SSL server + SParrot
- [ ] Process config file
- [ ] Multithreading
- [ ] TCP client
- [ ] HTTP structs
- [ ] HTTP server and request handlers
  - [ ] Default 404
  - [ ] Health
  - [ ] Static website
  - [ ] Redirect
  - [ ] Proxy
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
