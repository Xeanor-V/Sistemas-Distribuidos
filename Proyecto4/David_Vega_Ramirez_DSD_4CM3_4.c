#include <bits/stdc++.h>
using namespace std;


class spider
{
    public:
        pair<double,double> pos;
        pair<int,int> seguir;
        double velocidad;
        int id;
    spider(double, double, double, int);
    void setSeguir(pair<int,int>);
    double updatePos(void);
};

*****************************

#include "spider.hpp"

spider::spider(double x, double y, double velocidad, int id)
{
    this->pos = make_pair(x,y);
    this->velocidad = velocidad;
    this->id = id;
}

void spider::setSeguir(pair<int,int> seg)
{
    seguir = seg;
}

double spider::updatePos()
{
    double dx = abs(pos.first-seguir.first), dy = abs(pos.second - seguir.second);
    double dist = sqrt(dx*dx + dy*dy);
    if(seguir.first < pos.first) dx*=-1;
    if(seguir.second < pos.second) dy*=-1;
    double xmove = (dx/dist * velocidad); // hay que cuidar el modulo para que no se pase
    double ymove = (dy/dist * velocidad); // si esta cosa es mayor a la distancia, lo seteamos a pos
    pos.first += xmove;
    pos.second += ymove;
    return abs(xmove+ymove); 
}

*****************************

#include <bits/stdc++.h>
#include "gfx.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <unistd.h>
using namespace std;


int iniciar = 0;
int parar = 0;
int L,puerto;
const double ERROR = 1e-3;
vector< pair<int,int> > sp;
vector<bool> stop(4);


void draw_square(int x, int y)
{
    int s = 10;
    //for(int i = y-(s/2); i <= y+(s/2); i++)
    //gfx_line(x,i,x+s,i);
    gfx_line(x-s,y-s,x+s,y-s);
    gfx_line(x+s,y-s,x+s,y+s);
    gfx_line(x+s,y+s,x-s,y+s);
    gfx_line(x-s,y+s,x-s,y-s);
    return;
}

void choque()
{
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(i == j) continue;
            int dist = abs(sp[i].first - sp[j].first) + abs(sp[i].second - sp[j].second);
            if(dist <=2 )
            {
                stop[i] = stop[j] = true;
            }
        }
    }
}

bool Igual(double a, double b) 
{ 
    return fabs(a - b) < ERROR;
}

pair<int,int> getNumber(char *buffer)
{
    int a = -1, b = -1;
    bool flag = true;
    for(int i = 0; i < 1024; i++)
    {
        if(buffer[i] == '#') break;
        if(buffer[i] ==' ')
        {
            flag = false;
            continue;
        }
        if(flag)
        {
            if(a == -1) a = 0;
            a*=10;
            a+= buffer[i]-'0';
        }
        else
        {
            if(b == -1) b = 0;
            b*=10;
            b+= buffer[i]-'0';
        }
    }
    return make_pair(a,b);
}

void responder(int socket, int id)
{
    char buffer[40] = {0};
    int valread;

    //enviando configuracion inicial
    //Tam pantalla
    string s = to_string(L)+"#";
    strcpy(buffer,s.c_str());
    cout<<"Enviando a " <<id<<' '<<buffer<<'\n';
    send(socket , buffer,40, 0);
    fill(buffer,buffer+40,0);

    //Id del cliente
    s = to_string(id)+"#";
    strcpy(buffer,s.c_str());
    cout<<"Enviando a "<<id<<' '<<buffer<<'\n';
    send(socket, buffer,40, 0);
    fill(buffer,buffer+40,0);

    //Regresa posicion inicial
    cout<<"Recibiendo a "<<id<<' ';
    valread = read( socket , buffer, 40);
    sp[id] = getNumber(buffer);
    cout<<sp[id].first<<' '<<sp[id].second<<endl;
    iniciar++;
    fill(buffer,buffer+40,0);
    cout<<"iniciar: "<<iniciar<<endl;
    //Esperando a que todas las arañas esten configuradas
    while(iniciar<4) usleep(100);
    //enviando posicion araña a seguir
    //pintando en pantalla
    int idT = (id-1);
    if(idT < 0) idT = 3;
    s = to_string(sp[idT].first) + " " + to_string(sp[idT].second) +"#";
    strcpy(buffer,s.c_str());
    cout<<"Enviando a "<<id<<' '<<buffer<<'\n';
    send(socket, buffer,40, 0);
    fill(buffer,buffer+40,0);
    while(true)
    {
        valread = read( socket , buffer, 40);
        sp[id] = getNumber(buffer);
        fill(buffer,buffer+40,0);
        usleep(41666);
        //cout<<id<<" - "<<sp[id].first<<' '<<sp[id].second<<endl;
        if(stop[id]) 
        {
            s = "1 9999#";
            strcpy(buffer,s.c_str());
            //cout<<"Enviando a "<<id<<' '<<buffer<<'\n';
            send(socket, buffer,40, 0);
            break;
        }
        //Id del cliente
        /*while(stop[idT])
        {
            idT= (idT-1);
            if(idT < 0) idT= 3;
        }*/
        s = to_string(sp[idT].first) + " " + to_string(sp[idT].second)+"#";
        strcpy(buffer,s.c_str());
        //cout<<"Enviando a "<<id<<' '<<buffer<<'\n';
        send(socket, buffer,40, 0);
        fill(buffer,buffer+40,0);

    }
    close(socket);
    //enviar señal de fin
}



void listener()
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[40] = {0};
      
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }  
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( puerto );    
    // Forcefully attaching socket to the puerto 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }


    // aceptar clientes
    vector<thread> clientes;
    for(int i = 0; i < 4 ; i++)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                           (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        cout<<"Cliente "<<i+1<<" conectado \n";
        clientes.push_back(thread(responder,new_socket,i));
    }
    while(parar <4)
    {
        parar = 0;
        gfx_clear();
        usleep(41666);
        for(int i = 0; i< 4; i++)
        {
            cout<<i<<" : "<<sp[i].first<<' '<<sp[i].second<<'\n';
            if(stop[i])parar++;
            draw_square(sp[i].first,sp[i].second);
            
        }
        choque();
        gfx_flush();
    }
    for(int i = 0; i < 4; i++) clientes[i].join();
    /*valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer );
    send(new_socket , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");*/
    //Esperar
    cin>>iniciar;
}

int main()
{
    sp = vector< pair<int,int> >(4);
    //cout<<"Puerto: ";
    puerto=8080;
    cout<<"L: ";
    cin>>L;
    gfx_open(L, L, "Seguidores");
    gfx_color(0,200,100);
    //draw_square(100,100);
    listener();
}

*****************************

#include "spider.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080
pair<int,int> getNumber(char *buffer)
{
    int a = -1, b = -1;
    bool flag = true;
    for(int i = 0; i < 1024; i++)
    {
        if(buffer[i] == '#') break;
        if(buffer[i] ==' ')
        {
            flag = false;
            continue;
        }
        if(flag)
        {
            if(a == -1) a = 0;
            a*=10;
            a+= buffer[i]-'0';
        }
        else
        {
            if(b == -1) b = 0;
            b*=10;
            b+= buffer[i]-'0';
        }
    }
    return make_pair(a,b);
}
  
int main(int argc, char const *argv[])
{
    int Velocidad;
    cout<<"Velocidad: ";
    cin>>Velocidad;

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[40] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  

    // conectarse al servidor e iniciar 

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Leer L y definir posicion inicial
    //send(sock , hello , strlen(hello) , 0 );
    valread = read( sock , buffer, 40);
    pair<int,int> L = getNumber(buffer);
    fill(buffer,buffer+40,0);
    valread = read(sock, buffer,40);
    pair<int,int> id = getNumber(buffer);
    cout<<L.first<<' '<<id.first<<'\n';
    double x,y;
    switch(id.first)
    {
        case 0:
        x = 0;
        y = 0;
        break;
        case 1:
        x = L.first;
        y = 0;
        break;
        case 2:
        x = L.first;
        y = L.first;
        break;
        case 3:
        x = 0;
        y = L.first;
        break;
    }
    //Setear la spider del cliente
    spider current(x,y,Velocidad,id.first);
    string s = to_string((int) x) + " " + to_string((int)y)+"#";
    send(sock , s.c_str(), s.size() , 0 );
    //Enviar posicion inicial al servidor
    double res = 0;
    pair<int,int> op;
    while(true)
    {
        fill(buffer,buffer+40,0);
        valread = read(sock, buffer,40);
        pair<int,int> aux = getNumber(buffer);
        if(aux.first <= L.first && aux.first>=0)
        {
            op.first = aux.first;
            op.second = aux.second;
        }

        //cout<<"Seguir: "<<op.first<<' '<<op.second<<'\n';
        if(op.second == 9999) // Choco
        {
            break;
        }
        //updateamos posicion a seguir
        current.setSeguir(op);
        res += current.updatePos();
        //regresamos posicion actual
        string s = to_string((int)current.pos.first)+ " " + to_string((int)current.pos.second) + "#";
        //cout<<"Posicion: "<<s<<'\n';
        send(sock , s.c_str(), s.size() , 0 );
    }
    cout<<"Distancia recorrida: "<<res<<'\n';
    close(sock);
    return 0;
}

*****************************

//Comandos para compilar servidor
    g++ Server.cpp -std=c++11 -pthread -c
    g++ gfx.o Server.o -o Server -lX11 -pthread
//Comandos para compilar cliente
    g++ Cliente.cpp spider.cpp spider.hpp -std=c++11
//Ejecutar
    Servidor -> ./Servidor
    Cliente ->  ./a.out

