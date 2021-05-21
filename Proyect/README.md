# Comandos para intentar correr proyecto.

Se deben importar las siguientes librerias:
``` export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH```
```export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH```

Una vez importas las librerias, se puede compilar con las siguientes formas:
```g++ -g -Wall server.cpp new.pb.cc -o s -lprotobuf -lpthread```
```g++ -g -Wall client.cpp new.pb.cc -o c -lprotobuf -lpthread```

o bien
```g++ -g -Wall server.cpp new.pb.cc -o s `pkg-config --cflags --libs protobuf` -lpthread```
```g++ -g -Wall client.cpp new.pb.cc -o c `pkg-config --cflags --libs protobuf` -lpthread```

Una vez compilados, si se logra, ejecutar el servidor con el siguiente comando:
```./s <port>``` 

Y ejecutar el cliente
```./c <ip> <port> <user> <local-ip>```

Al momento, el cliente unicamente puede conectarse al servidor, registrar su nombre e intentar enviar un mensaje. Sin embargo, el mensaje no se puede recibir correctamente de parte del servidor.
