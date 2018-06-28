# SCU Web Server 

Jackson Beadle
COEN 317 - Distributed Systems
Programming Assignment 1 
Due date: 2/21/18

### Description

webserver.c is an HTTP server for the SCU homepage. It takes a document 
root, i.e., where all resource files are located, and a port number to 
listen on as command line parameters. This program uses socket programming
to listen for incoming connections, reads an HTTP request for a resource, 
and sends an HTTP response back to the client.

webserver.c can handle multiple connections simultaneously. It currently
supports HTTP response codes 200, 400, 403, and 404. Once a connection is
accepted and a request is received, the program forks then reads the request
to check if it is well-formed. If not, or if the client requests an
unsupported file type, a 400 code is sent back. If the resource requested
exists, but cannot be opened, a 403 code is sent back. If the file is not
found in the document root, a 404 code is sent back. Otherwise, the file
exists and is available to send with a 200 code to the client. In this case,
the file is opened, read in iteratively with a buffer, and written to the
client.

Currently supported file types:

- .html
- .txt
- .css
- .js
- .jpg
- .gif
- .svg
- .png

To compile: gcc -o server webserver.c
Usage: ./server -document_root </path/to/folder> -port <portno>

The document root should hold index.html and the folder with the remaining
resources necessary for the SCU homepage.

The SCU homepage files are located in the 'scu_files' folder. 

The TAR file submitted contains:

- README.txt (this file)
- webserver.c
- screenshots (directory w/ screenshots of browser)
