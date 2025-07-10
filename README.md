<h1>tiramisu</h1>
amazing CLI to deploy without cloud infrastructure without relying on the madness of kubernetes

SCOPE: \
    - basically, this CLI will install a remote webservice using ssh connection.
    After that, will compile any passed program into a wasm executable.
    Than you can deploy the wasm executable in the server.
    The executable will be reachable using its path.
    The path will be in the form of url/path/executable.
    The executable must be treated as an https request.

commands: \
SETUP # install everything needed to run tiramisu web server by executing an installation script on remote host
BUILD:
    - build files into wasm executable

DEPLOY: \
    - folders to be deployed (can be recoursive)

WEBSERV: \
    - connect 'url' \
    - install # install the webservice and the wasi runtime \
    - start # start the webservice \
    - restart # restart webservice \
    - shutdown # shutdown web service \
    - logs # show webservice logs
    - status
    - configure
    - reload
    - unistall
    - test-config

you can register an host to use it later using its alias
HOST:
 - add # add a new host to the hosts list
 - list # list all stored hosts

emcc: emcc -sSTANDALONE_WASM=1 -sPURE_WASI=1 file.c/cpp -o file.wasm


TODO:
    - environment var load
    - parse/update config.yaml
    - install wasmtime remotely