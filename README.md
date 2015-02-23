# TCP-Server-Client-in-C-Cpp
Implemented both a TCP client for retrieving files, and a TCP server for serving the requested files. The client, after establishing a connection with the server, sends over the name of the file it requests. Then, the server searches for the file in its memory cache followed by a predefined directory. If the file is found in either of the places, the server then transmits the content of file back to the client via the same connection, and caches the file content, if it is not inside the memory cache. The size limitation of the memory cache is 64MB. If the file size is more than 64MB on disk, you do not cache it in memory. And if the cache size will exceed 64MB after putting the file into memory, you have to purge enough cached content to make sure the entire cache size is restricted to 64MB anytime. 


for example:

% ./tcp_server 9999 /home/workspace/test.html
Client 159.78.55.65 is requesting file test.html
Cache miss. test.html sent to the client
Client 159.78.55.66 is requesting file test.html
Cache hit. test.html sent to the client
Client 159.78.55.66 is requesting file test.html
Cache miss. test.html sent to the client
Client 159.78.55.65 is requesting file test.html
Cache hit. test.html sent to the client

% /tcp_client 159.78.58.28 9999 test.html .
File test.html saved
