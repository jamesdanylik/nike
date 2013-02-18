The Plan
========

After a long failed attempt showing the limitations of processes, we elect to
port our partially functioning model to pthreads.  Threaded exeectution will
require the creation of two thread-safe data structures, CxnCache and PageCache,
which will be uses to store/reuse connections and cache pages respectively.
The cache will be protected by a mutex and be the only thing allowed to retrieve
remote pages in order to prevent errors.

Our server will start a main thread to allocate a listen port and then wait for
requests upon it, creating a new thread to handle each connection up to a max of
ten.  Each thread will listen for incoming bytes on its connection, looking for
the two /r/n that mark the end of a HttpRequest.  Once found, we parse the header
and handle errors, and then read the optional content-length that follows if it
exists.  

Next, we check if this request, identified by its url, is in the cache.  If it
is, serve it directly; else, get it from the remote server, attempting to reuse
a connection if possible.  Finally, send this response to the client and return
to listening for bytes.
