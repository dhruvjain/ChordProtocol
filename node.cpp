#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "dirent.h"
#include <signal.h>
#include <unistd.h>
#include "fcntl.h"
#include "sys/stat.h"

using namespace std;

#define TRUE 1
#define FALSE 0
#define EQUAL(X,Y) (strcmp(X, Y)==0?1:0)
#define FSIZE 64;

string servername="10.132.132.68";
string myip;
string prec_ip=servername;
string succ_ip=servername;
bool update_table_flag=false;
bool update_quit_flag=false;
int PORT1,PORT2;

pid_t pid,pid2,pid3;

int nnodes=1;
map<string,string> hashing; // file_name,machine_name
vector<string> files_db;
string dir;

int m=63;


void error(const char* msg)
{
    perror(msg);
    exit(1);
}

#define  ull unsigned long long
#define  mp make_pair 
using namespace std;
#define MAXLEN 10

typedef struct finger_table
{
    //string start;
    string succ;
    ull start;
    ull suc;
    
}finger_table;
finger_table table[63];

ull stringtoull(string str){
    int i = 0;
    ull num=0;
    for(i=0;i<str.size();i++){
        num = 10*num + (str[i]-'0');
    }
    return num;
}


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
sockaddr_in getaddr(string server_name,int port){


    sockaddr_in serv_addr;
   
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_aton(server_name.c_str(), &serv_addr.sin_addr)!=0) {
        inet_pton(AF_INET, server_name.c_str(), &serv_addr.sin_addr);
    }

    return serv_addr;



}
std::vector<std::string> getfiles(std::string directory)
{
    
    std::vector<std::string> v;
    DIR* dp;
    struct dirent* ep;
    dp=opendir(directory.c_str());

    if(dp!=NULL)
    {
        while((ep=readdir(dp))){
            if(!EQUAL(ep->d_name,"..") && !EQUAL(ep->d_name,".")&&ep->d_name[0]!='.')
                v.push_back(ep->d_name );
        }
        closedir(dp);
    }
    else 
        error("could not open Directory");
    return v;
}
ull oat_hash(string p)
{
    
    ull h = 0;
    int i;
    int len=p.length();
    for (i = 0; i < len; i++)
    {
        h += p[i];
        h += (h << 10);
        h ^= (h >> 6);
    }

    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);

    return h;
}
int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}
class Client{
    
public:
    struct sockaddr_in myaddr, serv_addr;
    int fd, i, slen;
    char buf[10000];    /* message buffer */
    int recvlen;
    char *server_name;
    int portno,type;
    Client(int socktype,int port,std::string servername)
    {
        /* # bytes in acknowledgement message */
        slen=sizeof(serv_addr);
        type=socktype;
        /* create a socket */

        if ((fd=socket(AF_INET, socktype, 0))==-1)
            printf("error in socket created\n");

        /* bind it to all local addresses and pick any port number */

        memset((char *)&myaddr, 0, sizeof(myaddr));
        myaddr.sin_family = AF_INET;
        myaddr.sin_addr.s_addr = inet_addr("");
        myaddr.sin_port = htons(0);

        if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
            perror("bind failed");
        }
        

        memset((char *) &serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        if (inet_aton(servername.c_str(), &serv_addr.sin_addr)!=0) {
            inet_pton(AF_INET, servername.c_str(), &serv_addr.sin_addr);
        }
        server_name=strdup(servername.c_str());
        portno=port;
        
    }
    Client(int socktype,int port,in_addr_t addr)
    {
        slen=sizeof(serv_addr);
        type=socktype;
        /* create a socket */

        if ((fd=socket(AF_INET, socktype, 0))==-1)
            printf("socket created\n");

        /* bind it to all local addresses and pick any port number */

        memset((char *)&myaddr, 0, sizeof(myaddr));
        myaddr.sin_family = AF_INET;
        myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        myaddr.sin_port = htons(0);

        if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
            perror("bind failed");
        }


        memset((char *) &serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr=addr;
        portno=port;
    }
    int send_data(std::string data,string servip)
    {
        memset((char *) &serv_addr, 0, sizeof(serv_addr));

        serv_addr=getaddr(servip,portno);
        // printf("Sending packet %d to %s port %d\n", i, server_name, portno);
        
        int res;
        if(type==SOCK_DGRAM){
            if ((res=sendto(fd, data.c_str(), data.size()+1, 
                            0, (struct sockaddr *)&serv_addr, slen))==-1) {
                error("sendto");
            }
        }
        else{
            if((res=send(fd, data.c_str(), data.size()+1, 0))<0)
                error("send_data:");
        }
        return res;
    }
    std::string receive_data()
    {
        int nbytes;
        bzero(buf,10000);
        socklen_t leng=sizeof(serv_addr);
        if(type==SOCK_DGRAM)
            nbytes = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&serv_addr, &leng);
        else
            nbytes=recv(fd, buf, sizeof(buf), 0);
        if (nbytes<0)
        {
            error("receive_data:");
            sprintf(buf, "0");
        }
        return std::string(buf);
    }
    std::string read_data_ncopy(string name)
    {
        int n_chars;
        int wfd=creat(name.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
        int filesize=atoi((receive_data()).c_str());
        //printf("filesize=%d\n",filesize );
        int tsize=0,prevsize=0;
        while( (n_chars = read(fd, buf, sizeof(buf))) > 0 )
        {
            buf[n_chars+1]='\0';
            
            if( write(wfd, buf, n_chars) != n_chars )
            {
              printf("error writing\n");
            }
            else{
                tsize+=n_chars;
                if(tsize==filesize){
                    cout<<"FILE RECEIVED !!!";
                }
                if((int)(tsize*100/filesize)!=prevsize)
                {
                    printf("%f done\n ",tsize*1.0/filesize*100 );
                    prevsize=tsize*100/filesize;
                }
                    

            }
                
         
         
            if( n_chars == -1 )
            {
              printf("error reading\n");
            }
        }
        close(wfd);
        printf("written successfully to %s\n",name.c_str());
        return string(buf);
    }
    int connect_toserver()
    {
        int res=connect(fd, (sockaddr*)&serv_addr, slen);
        if(res<0)
            error("connect_toserver:");
        printf("connected to server at %u\n",serv_addr.sin_addr.s_addr);
        return res;
    }
    int close_conn()
    {

        close(fd);
    }
};




class PeerServer{
    
public:
    int sockfd,newfd,newsockfd,type;
    sockaddr_in serv_addr,cli_addr;
    socklen_t clilen;
    std::map<std::string, in_addr_t> database;
    char buff[1000];
    PeerServer(int socktype,int port)
    {
        
        socklen_t clilen;
        char buffer[256];
        
        int n;

        sockfd = socket(AF_INET, socktype, 0);
        if (sockfd < 0) 
            error("ERROR opening socket");
        type=socktype;

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
                                sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
        
    }
    int start_listening(int waitno)
    {
        listen(sockfd, waitno);
        clilen = sizeof(cli_addr);
        return 0;
    }
    int accept_new()
    {
        if(type==SOCK_DGRAM)
            return 0;
        newsockfd=accept(sockfd, (sockaddr*)&cli_addr, &clilen);
        if(newsockfd<0)
            error("accept:");
        return newsockfd;
    }
    /*for sock_stream*/
    int receive_data()
    {
        bzero(buff,1000);
        int res=recv(newsockfd, buff, sizeof(buff), 0);
        if(res<0)
            error("receive_data:");
        return res;
    }
    /*for datagram*/
    int receive_data(sockaddr* addr,socklen_t* len)
    {
        int nbytes=recvfrom(sockfd, buff, sizeof(buff), 0,
                    (sockaddr*)&cli_addr,len);
        if(nbytes<0)
            error("receive_data(int):");
        return nbytes;
    }
    /*for sock stream*/
    int send_data(std::string data)
    {
        int res=send(newsockfd, data.c_str(), data.size()+1, 0);
        if(res<0)
            error("send_data:");
        return res;
    }
    int send_data(int t)
    {
        char temp[50];
        sprintf(temp, "%d",t);
        return send_data(temp);
    }
    /*for datagram*/
    int send_data(std::string data,sockaddr* addr,socklen_t &len)
    {
        int res=sendto(newsockfd,data.c_str(), data.size()+1, 0, 
                addr, len);
            if(res<0)
                error("send_data(int):");
        return res;
    }
    void server_process()
    {
        fd_set masterr,masterw,readfds,writefds;
        FD_ZERO(&masterr);
        FD_ZERO(&masterw);
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(sockfd, &masterr);
        std::set<std::pair<int,int> > rwsets;
        int maxfd=sockfd;
        
        while(1)
        {

            timeval time_out;
            time_out.tv_sec=1;
            time_out.tv_usec=1000;
            readfds=masterr,writefds=masterw;
            if(select(maxfd+1, &readfds, &writefds, NULL,NULL )==-1)
                error("select:");

            if(FD_ISSET(sockfd, &readfds)){
                //printf("FD_SET true\n");
                newsockfd=accept_new();
                receive_data();
                printf("new client asked for file %s!!\n",buff);
                std::string f_name(buff);

                string dirname=dir;
                string filename=dirname+"/"+f_name;
                cout<<"full path :"<<filename<<endl;


                int readfd=open(filename.c_str(), O_RDONLY);
                if(readfd<0){
                    perror("Error in opening file\n");
                    close(newsockfd);
                    continue;
                }
                struct stat st;
                stat(filename.c_str(),&st);
                send_data(st.st_size);
                //printf("insert done\n");
                rwsets.insert(std::make_pair(newsockfd, readfd));
                FD_SET(newsockfd, &masterw);
                maxfd=std::max(newsockfd,maxfd);
            }
            
            std::set<std::pair<int,int> >::iterator it;
            std::vector<std::pair<int,int> > rmv;
            for(it=rwsets.begin();it!=rwsets.end();it++)
            {
                int nbytes=read(it->second, buff, sizeof(buff));
                if(nbytes==0)
                {
                    close(it->second);
                    //printf("transferred file\n");
                    close(it->first);
                    
                    std::pair<int,int> rmp=*it;
                    
                    FD_CLR(it->first, &masterw);
                    rmv.push_back(rmp);
                }
                else{
                    if(write(it->first, buff, nbytes)!=nbytes)
                        printf("write error to client\n");
                }
            }
            for(int i=0;i<rmv.size();i++)
                rwsets.erase(rmv[i]);
        }
    }
};

class Server{
    
public:
    int sockfd,newfd,newsockfd,type;
    sockaddr_in serv_addr,cli_addr;
    socklen_t clilen;

    char buff[1000];
    Server(int socktype,int port)
    {
        
        socklen_t clilen;

        int n;

        sockfd = socket(AF_INET, socktype, 0);
        if (sockfd < 0) 
            error("ERROR opening socket");
        type=socktype;

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
                                sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
        
    }
    int start_listening(int waitno)
    {
        listen(sockfd, waitno);
        clilen = sizeof(cli_addr);
        return 0;
    }
    int accept_new()
    {
        if(type==SOCK_DGRAM)
            return 0;
        newsockfd=accept(sockfd, (sockaddr*)&cli_addr, &clilen);
        if(newsockfd<0)
            error("accept:");
        return newsockfd;
    }
    /*for sock_stream*/
    int receive_data()
    {   bzero(buff,1000);
        int res=recv(sockfd, buff, sizeof(buff), 0);
        if(res<0)
            error("receive_data:");
        return res;
    }
    /*for datagram*/
    int receive_data(sockaddr* addr,socklen_t* len)
    {
        bzero(buff,1000);
        int nbytes=recvfrom(sockfd, buff, sizeof(buff), 0,
                    (sockaddr*)&cli_addr,len);
        if(nbytes<0)
            error("receive_data(int):");
        return nbytes;
    }
    /*for sock stream*/
    int send_data(std::string data)
    {
        int res=send(sockfd, data.c_str(), data.size()+1, 0);
        if(res<0)
            error("send_data:");
        return res;
    }
    /*for datagram*/
    int send_data(std::string data,sockaddr* addr,socklen_t &len)
    {
        int res=sendto(sockfd,data.c_str(), data.size()+1, 0, 
                addr, len);
            if(res<0)
                error("send_data(int):");
        return res;
    }



    void process(Client c){


        char buff[1000];
        
        sockaddr_in clientaddr;
        cout<<"s running \n"<<endl;
        while(1){
            bzero(buff,1000);
            int nbytes=recvfrom(sockfd, buff, sizeof(buff), 0,(sockaddr*)&cli_addr,&clilen);

            if(nbytes<0)
                error("server_process:");

            string s(buff);

            //cout<<"received "<<s<<endl;

            if(s.substr(0,4) == "NODE"){

                string ip=s.substr(5);
                nnodes++;
                
                ull mid=oat_hash(ip);
                ull id=oat_hash(myip);
                ull succ_id=oat_hash(succ_ip);
                ull prec_id=oat_hash(prec_ip);

                memset((char *) &clientaddr, 0, sizeof(clientaddr));
                clientaddr.sin_family = AF_INET;
                clientaddr.sin_port = htons(PORT1);
                clientaddr.sin_addr.s_addr = inet_addr(ip.c_str());

                cout<<inet_ntoa(clientaddr.sin_addr)<<endl;



                if(mid>=id){ // succ side
                
                    if(nnodes==2||mid<succ_id){


                        string sendbuffer="SCPR#"+succ_ip+"#"+myip;
                        
                        int res=sendto(sockfd,sendbuffer.c_str(),sendbuffer.length(),0,(const sockaddr*)&clientaddr,sizeof(clientaddr));
                        
                    }
                    else{ // mid>succ_id
                        
                        c.send_data(s,succ_ip);
                    }

                } 
                else{ // pred side
                
                    if(nnodes==2|| mid>prec_id){
                        
                        string sendbuffer="SCPR#"+myip+"#"+prec_ip;
                        
                        int res=sendto(sockfd,sendbuffer.c_str(),sendbuffer.length(),0,(const sockaddr*)&clientaddr,sizeof(clientaddr));
                        

                    }
                    else{ // mid<prec_id

                        c.send_data(s,prec_ip);
   
                    }


                }

            }
            else if(s.substr(0,4) == "SUCC"){

                succ_ip=s.substr(5);
                cout<<"my succ is now "<<succ_ip<<endl;

            }
            else if( s.substr(0,4)== "PRED"){
                prec_ip=s.substr(5);
                cout<<"my prec is now "<<prec_ip<<endl; 
                ull id=oat_hash(prec_ip);

                std::map<string, string>::iterator it;
                for(it=hashing.begin();it!=hashing.end();it++){

                    ull fid=oat_hash(it->first);

                    if(fid<=id){

                        string sbuff="FILE#"+it->first+"#"+it->second;
                        memset((char *) &clientaddr, 0, sizeof(clientaddr));
                        clientaddr.sin_family = AF_INET;
                        clientaddr.sin_port = htons(PORT1);
                        clientaddr.sin_addr.s_addr = inet_addr(prec_ip.c_str());
                        int res=sendto(sockfd,sbuff.c_str(),sbuff.length(),0,(const sockaddr*)&clientaddr,sizeof(clientaddr));
                        hashing.erase(it);
                    }

                }



            }
            else if(s.substr(0,4)=="FILE"){

                string recvstr=s.substr(5);
                std::vector<string> fip=split(recvstr,'#');

                ull fid=oat_hash(fip[0]);

                ull mid=oat_hash(myip);

                ull succ_id=oat_hash(succ_ip);
                ull prec_id=oat_hash(prec_ip);


                if(mid>=fid && prec_id<=fid){

                    hashing[fip[0]]=fip[1];// filename-->ip of file owner
                    cout<<"I will keep "<<fip[0]<<endl;

                }

                else if((fid>prec_id && fid>mid && mid<prec_id && mid<succ_id )|| (fid<prec_id && fid<mid && mid<prec_id && mid<succ_id )){

                    hashing[fip[0]]=fip[1];
                    cout<<"I will keep "<<fip[0]<<endl;
                    // machine found at critical condition
                }
                else{


                    c.send_data(s,succ_ip);
                    cout<<"I will send  "<<fip[0]<<"  to "<<succ_ip<<endl;
                    // send to succesor
                }

    
            }   
            else if(s.substr(0,4)=="REQU"){


                string recvstr=s.substr(5);
                //cout<<"recv "<<recvstr<<endl;
                std::vector<string> fip=split(recvstr,'#');
                memset((char *) &clientaddr, 0, sizeof(clientaddr));
                clientaddr.sin_family = AF_INET;
                clientaddr.sin_port = htons(PORT1);
                clientaddr.sin_addr.s_addr = inet_addr(fip[1].c_str());

                
                // for(int i=0;i<m;i++){

                //         cout<<table[i].start<<"and "<<table[i].succ<<endl;
                // }
                if(hashing.find(fip[0])==hashing.end()){// not found
                    //cout<<"not found \n sent to "<<succ_ip<<endl;
                    //c.send_data(s,succ_ip);
                    ull fid=oat_hash(fip[0]);
                    
                    for(int i=0;i<m;i++){

                        if(oat_hash(table[i].succ)==fid){
                            //string sendbuffer="TABLE_SUCC#"+idnum+"#"+table[i].succ;
                            c.send_data(s,table[i].succ);
                            //cout<<"snet to 1 "<<table[i].succ<<endl;
                            break;
                        }
                        // else if(i==0 && oat_hash(table[i].succ)>fid){

                        //     c.send_data(s,prec_ip);
                        //     cout<<"snet to 2"<<table[i].succ<<endl;
                        //     break;

                        // }
                        else if(i==m-1){
                            
                            c.send_data(s,table[i].succ);
                            //cout<<"snet to 3"<<table[i].succ<<endl;
                            break;
                        }
                        else if(oat_hash(table[i].succ)<fid && oat_hash(table[i+1].succ)>fid){

                            //string sendbuffer="TABLE_SUCC#"+idnum+"#"+table[i].succ;
                            c.send_data(s,table[i].succ);
                            //cout<<"snet to 4"<<table[i].succ<<endl;
                            break;
                            
                        }
                    }

                }
                else{// found

                    string sendbuffer="FNAME#"+hashing[fip[0]]+"#"+fip[0];
                    //cout<<"FNAME: "<<fip[0]<<":"<<sendbuffer<<endl;
                    int res=sendto(sockfd,sendbuffer.c_str(),sendbuffer.length(),0,(const sockaddr*)&clientaddr,sizeof(clientaddr));
                
                }

            }
            else if(s.substr(0,4)=="SCPR"){
                string res=s.substr(5);

                //cout<<"res is "<<res<<endl;

                vector<string> x = split(res, '#');
                succ_ip=x[0];
                prec_ip=x[1];
                
                string buff="";
                buff="PRED#"+myip;
                c.send_data(buff,succ_ip);
                buff="";
                buff="SUCC#"+myip;
                c.send_data(buff,prec_ip);

                // send files to servername
                for(int i=0;i<files_db.size();i++){

                    //cout<<files_db[i]<<endl;
                    c.send_data("FILE#"+files_db[i],servername);
                }

                // update start of table
                for(int i=0;i<m;i++){

                    ull st=(oat_hash(myip)+(ull)pow(2,i))%(ull)pow(2,m);
                    table[i].start=st;
                    char buff[1000];
                    sprintf(buff,"%lld",st);
                    string sendbuffer="GETS#"+string(buff)+"#"+myip;
                    c.send_data(sendbuffer,servername);
                    bzero(buff,1000);
                }

                string sendbuffer="UPDATE_TABLE#"+myip;
                c.send_data(sendbuffer,servername);
                cout<<"sent update table "<<endl;


            }
            else if(s.substr(0,4)=="GETS"){

                string buff=s.substr(5);

                vector<string> x = split(buff, '#');

                string idnum=x[0];
                string ip=x[1];

                ull fid=stringtoull(idnum); // target id

                ull mid=oat_hash(myip);

                ull succ_id=oat_hash(succ_ip);
                ull prec_id=oat_hash(prec_ip);

                if(mid>=fid && prec_id<=fid){
                        
                        string sendbuffer="TABLE_SUCC#"+idnum+"#"+myip;
                        c.send_data(sendbuffer,ip);
         
                }

                else if((fid>prec_id && fid>mid && mid<prec_id && mid<succ_id )|| (fid<prec_id && fid<mid && mid<prec_id && mid<succ_id )){

                        string sendbuffer="TABLE_SUCC#"+idnum+"#"+myip;
                        c.send_data(sendbuffer,ip);
                        
                    // machine found at critical condition
                }
                else{


                    c.send_data(s,succ_ip);
                    
                    // send to succesor
                }


            }
            else if(s.substr(0,10)=="TABLE_SUCC"){

                    string buff=s.substr(11);

                    vector<string> x = split(buff, '#');

                    string idnum=x[0];
                    string s_ip=x[1];

                    ull id=stringtoull(idnum);

                    for(int i=0;i<m;i++){

                        if(table[i].start==id){
                            table[i].succ=s_ip;
                            break;
                        }

                    }


            }
            else if(s.substr(0,12)=="UPDATE_TABLE"){

                    string buff=s.substr(13);
                    //cout<<"Value of flag= "<<update_table_flag<<endl;
                    if(update_table_flag==false && myip!=buff){

                        ull id=oat_hash(buff);

                        for(int i=0;i<m;i++){
                            //cout<<"in ft \n";
                            cout<<table[i].start<<" "<<id<<" "<<table[i].succ<<" "<<oat_hash(table[i].succ)<<endl;

                            if(table[i].start<id && oat_hash(table[i].succ)>id){
                                //cout<<"updated "<<buff<<endl;
                                table[i].succ=buff;
                            }
                            else if(table[i].start>oat_hash(table[i].succ) && (id>table[i].start || id<oat_hash(table[i].succ))){
                                //cout<<"updated "<<buff<<endl;
                                table[i].succ=buff;

                            }
                        }
                         cout<<"UPDATED \n";

                        for(int i=0;i<m;i++){

                            cout<<table[i].start<<" "<<id<<" "<<table[i].succ<<" "<<oat_hash(table[i].succ)<<endl;

                        }

                        c.send_data(s,succ_ip);
                        update_table_flag=true;

                    }

                    else{
                        if(update_table_flag!=true)
                            c.send_data(s,succ_ip);
                        update_table_flag=false;
                    }


            }
            else if(s.substr(0,5)=="FNAME"){

                string buff=s.substr(6);

                //cout<<"recv "<<s<<endl;
                vector<string> x = split(buff, '#');

                Client c3(SOCK_STREAM,PORT2,x[0]);
                c3.connect_toserver();
                c3.send_data(x[1],x[0]);
                c3.read_data_ncopy(x[1]);
            }
            else if(s.substr(0,4)=="EXIT"){
            	c.send_data("QUIT#"+myip+"#"+succ_ip+"#"+prec_ip,succ_ip);

	            std::map<string, string>::iterator it;
	            //cout<<"size ="<<hashing.size()<<endl;
	            for(it=hashing.begin();it!=hashing.end();it++){

	                if(it->second==myip)
	                    hashing.erase(it);
	                else{
	                    c.send_data("FILE#"+it->first+"#"+it->second,succ_ip);
	                }
	            }
	            kill(pid, SIGKILL);
	            kill(pid2, SIGKILL);
	            exit(1);
            }

            else if(s.substr(0,4)=="QUIT"){

                string buff=s.substr(5);

                vector<string> x = split(buff, '#');

                string qip=x[0];
                string s_ip=x[1];
                string p_ip=x[2];
                if(update_quit_flag==false){
                	cout<<"RECEIVED QUIT \n"<<endl;
                    if(myip==s_ip){

                        prec_ip=p_ip;

                    }
                    else if(myip==p_ip){

                        succ_ip=s_ip;

                    }

                    std::map<string, string>::iterator it;
                    cout<<"siz of hash ="<<hashing.size()<<endl;
                    for(it=hashing.begin();it!=hashing.end();it++){

                        if(it->second==qip)
                            hashing.erase(it);
                    }
                    for(int i=0;i<m;i++){

                        if(table[i].succ==qip){

                            table[i].succ=s_ip;
                        }
                    }
                    c.send_data(s,succ_ip);
                    update_quit_flag=true;

                }
                else{
                    update_quit_flag=true;
                }


            }
    
        }
    }
    
};

string getmyip(){

    const char* google_dns_server = "8.8.8.8";
    int dns_port = 53;
     
    struct sockaddr_in serv;
     
    int sock = socket ( AF_INET, SOCK_DGRAM, 0);
     
    //Socket could not be created
    if(sock < 0)
    {
        perror("Socket error");
    }
     
    memset( &serv, 0, sizeof(serv) );
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr( google_dns_server );
    serv.sin_port = htons( dns_port );
 
    int err = connect( sock , (const struct sockaddr*) &serv , sizeof(serv) );
     
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*) &name, &namelen);
         
    char buffer[100];
    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);
         
    if(p != NULL)
    {
       
    }
    else
    {
        //Some error
        printf ("Error number : %d . Error message : %s \n" , errno , strerror(errno));
    }
 
    close(sock);
     
    return string(buffer);
}
int main(){


    string filename;
    int port;
    myip=getmyip();
    std::vector<std::string> v;

    cout<<"Enter udp port \n";
    cin>>PORT1;

    cout<<"Enter TCP port \n";
    cin>>PORT2;

    Server s(SOCK_DGRAM,PORT1);

    s.start_listening(5);

    PeerServer ps(SOCK_STREAM,PORT2);
    ps.start_listening(10);
    Client c(SOCK_DGRAM,PORT1,servername);

    string buff;

    //cout<<"Enter the directory to share : \n"; // for sharing file
    dir="/home/dhuruv/Desktop/imdb";
    //cin>>dir;
    v=getfiles(dir);

    for(int i=0;i<v.size();i++){

        string buff=v[i]+"#"+myip;
        files_db.push_back(buff);
        if(myip==servername){
            hashing[v[i]]=myip;
        }
    }

    cout<<"FILE saving done !!"<<endl;

    if(myip==servername){

        ull id=oat_hash(myip);

        for(int i=0;i<m;i++){
                //table[i].start=myname;
                ull st=(oat_hash(myip)+(ull)pow(2,i))%(ull)pow(2,m);
                table[i].start=st;
                table[i].succ=myip;
                //cout<<st<<"-->"<<myip<<endl;
        }   


    }

    pid=fork();
    if(pid!=0){ // parent
       
    }
    else{//child
        
        s.process(c); //udp server

    }
    


    string s1="NODE#";
    cout<<s1+"#"+myip<<endl;

    if(myip!=servername){
        buff="";
        buff=s1+myip;
        cout<<buff<<endl;
        c.send_data( buff,servername);
        cout<<"sent "<<buff<<endl;
 
    }

  
    
    pid2=fork();
    if(pid2==0){
            ps.server_process(); // file transfer tcpserver
            exit(1);
    }

    while(1){
        cout<<"Enter the filename you want to download (enter QUIT to leave)\n";
        cin>>filename;
        if(filename!="QUIT"){
            c.send_data("REQU#"+filename+"#"+myip,servername);
        }
        else{
        	cout<<"sent q to "<<succ_ip<<endl;
        	c.send_data("EXIT#"+myip,myip);
            


        }
    }

}