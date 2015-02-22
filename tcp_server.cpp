// header
#include "tcp_server.h"

//this function removes the first character of a char array
char *removeFirst (char *charNew) {
  char *str;
  
  if (strlen(charNew) == 0)
    str = charNew;
  else
    str = charNew + 1;
  return str;
}

//this function joins the directory and filename
std::string getFullDirect(char* direc, char *file)
{
    if((sizeof(direc) == 1) && (direc[0]=='.'))
    {
        std::string str1 = std::string(file);
        return str1;
    }
    else if ((sizeof(direc) == 2) && (direc[0]=='.') && (direc[0]=='/'))
    {
        std::string str1 = std::string(file);
        return str1;
    }
    else
    {
        std::string str1 = std::string(direc);
        std::string str2 = std::string(file); 
        str1.append("/");
        str1.append(str2);
    
        return str1;

    }
    std::string str1;
    return str1;
}

//this function returns the file size in a specific directory
size_t getFileSize(char* direc, char *file)
{
    struct stat f;
    
    std::string temp = getFullDirect(direc, file);
    
    if (stat(temp.c_str(),&f) != 0)
        return 0;
    return f.st_size;
    
}

//this function sends data to the client from the cache
// if the file is currently in the cache
void writeFromCache(char *file, Cache* cache, int writeSock)
{
    size_t totalSent = 0;
    std::string fileName(file);
    size_t sizeLeft = cache->list[fileName]->size;
    char * ptrFile = (char *) cache->list[fileName]->filePtr;
    size_t fileSize = sizeLeft;
    
    char temp[DATA_BLOCK];

    int errC;
    char fileSizeC[1024];
    sprintf(fileSizeC,"%lu",sizeLeft);

    // sending data and error checking on the data send
    errC = write(writeSock, fileSizeC, 1024);
    if (errC<0)
    {
        perror("Server Side Size writing error ");
        std::exit(EXIT_SUCCESS); 
    }

    while (totalSent < fileSize)
    {
        memset(temp, 0, DATA_BLOCK);
        
        size_t readSize = DATA_BLOCK;
        if (sizeLeft < DATA_BLOCK)
            readSize = sizeLeft; 

        memcpy(temp, ptrFile+totalSent , readSize);
        size_t lastSent = readSize;

        totalSent += lastSent;
        sizeLeft -= lastSent;
        
        errC = send(writeSock, temp, lastSent,0);
        
        if (errC<0)
        {
            perror("Server Side writing error from cache");
            std::exit(EXIT_SUCCESS); 
        }

    }
}

// this function checks if a program is in the cache
// if it is in the cache calls the writeFromCache function
bool inCache(char *file, Cache* cache, int writeSock)
{   
    if(!cache->inCache(file))
        return false;

    writeFromCache(file, cache, writeSock);
    return true;
}
   
// this function checks if a program is in the directory
// if it is in the directory then it sends the data to the client
bool inDirec(char* directory, char *file, int writeSock, Cache* cache)
{   
    DIR *dir = NULL;
    struct dirent *dirCont = NULL;
    
    char *dirFormat = directory;
    std::string sFileName = std::string(file);
    
    
    if (directory[0] =='/')
        dirFormat = removeFirst(directory);
    
    
    dir = opendir(dirFormat);
    if (dir == NULL)
    {
        perror("Couldn't Open Directory: ");

        std::exit(EXIT_SUCCESS);           
    }
    
    while ((dirCont = readdir(dir)) !=NULL)
    {

        if (strcmp(file,dirCont->d_name) == 0) {
            
            size_t fileSize = getFileSize(dirFormat, dirCont->d_name);
            bool cacheFlag = false;
            
            FILE *inFile;
            
            //checking if file is opened properly
            inFile = fopen((getFullDirect(dirFormat, dirCont->d_name)).c_str(), "rb");
            if (inFile == NULL)
            {
                perror("file error");
                std::exit(EXIT_SUCCESS);
            }
                                 
            size_t totalSent = 0;
            size_t sizeLeft = fileSize;

            char temp[DATA_BLOCK];
            
            int errC;
            char fileSizeC[1024];
            sprintf(fileSizeC,"%lu",sizeLeft);
            
            errC = write(writeSock, fileSizeC, 1024);
                
            if (errC<0)
            {
                perror("Server Side Size writing error");
                std::exit(EXIT_SUCCESS);  
            }
            
            
            
            while (totalSent < fileSize)
            {
                memset(temp, 0, DATA_BLOCK);
                
                //check if file is read well
                size_t lastSent = fread(temp, 1,sizeof(temp), inFile);
  
                if (lastSent > sizeof(temp) && lastSent < 0)
                {
                    perror("File reading error");
                    std::exit(EXIT_SUCCESS);  
                }
                
                totalSent += lastSent;
             
                errC = send(writeSock, temp, lastSent,0);
                
                if (errC<0)
                {
                    perror("Server Side writing error");
                    std::exit(EXIT_SUCCESS);  
                }
                
            }

            // close the file
            fclose(inFile); 
            //Cache check
            if (fileSize < CACHE_SIZE)
            {
                
                cacheFlag = true;
                cache->arrangeSize(fileSize);
                
                FILE *cFile = fopen((getFullDirect(dirFormat, dirCont->d_name)).c_str(), "rb");
                char *ptr; 
                ptr = (char *) malloc(sizeof(char)* (fileSize+1)); 
                
                //check if file is read well
                size_t result = fread(ptr, 1, fileSize, cFile);
                if (result > fileSize || result <0)
                {
                    perror("File reading error2 ");
                    std::exit(EXIT_SUCCESS);  
                }
                ptr[fileSize] = '\0';
                
                cache->add(sFileName, fileSize, ptr);
                
                fclose(cFile);
            }
            
            return true;
        }
        
    }
    
    //close file 
    closedir(dir);
    return false;
}
void dirExist(char* dir)
{

    DIR *d = NULL;
    
    char *dirFormat = dir;
    
    if (dir[0] =='/')
        dirFormat = removeFirst(dir);
    
    d = opendir(dirFormat);
    if (d == NULL)
    {
        perror("Couldn't Open Directory: ");

        std::exit(EXIT_SUCCESS);           
    }
    
}

/*
 * 
% tcp_server port_to_listen_on file_directory
e.g.
% ./tcp_server 9089 /home/dist/homework1
 */
int main(int argc, char**argv) {
    // Prints welcome message...
    std::cout << "Welcome To TCP Server..." << std::endl;

    // Checks correct number of argument if not error message and quit
    if (argc != 3) { 
        std::cout << std::endl << "2 Arguments expected:" << std::endl;
        for (int i = 1; i < argc; i++) {
            std::cout << i << ": " << argv[i] << std::endl;
        }
        std::cout <<"./tcp_server port_to_listen_on file_directory"<<std::endl;
        
        std::exit(EXIT_SUCCESS);
    }
    
    int socket_des, port, bind_status, listen_status, accept_status; 
    socklen_t clientLen;
    struct sockaddr_in server_add, client_add;
    char directory[512];
    Cache *cache = new Cache();
    
    //check directory size
    if (sizeof(argv[2])>512)
    {
        std::cout <<"Error: Directory should be less than 512 characters"<<std::endl;
        std::exit(EXIT_SUCCESS); 
    }
    
    //check port number
    port = atoi(argv[1]);
    if (port <=0 || port >=100000)
    {
        std::cout <<"Error: port no should be greater than zero and less than 100,000 "<<std::endl;
        std::exit(EXIT_SUCCESS); 
    }
    
    strcpy(directory, argv[2]);
    dirExist(directory);
    
    //check for socket starting error
    socket_des = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_des == -1){
        perror("Server Socket Error ");
        std::exit(EXIT_SUCCESS);   
    }
    
    memset(&server_add, '\0', sizeof(server_add));
    server_add.sin_family = AF_INET;
    server_add.sin_addr.s_addr = INADDR_ANY;
    server_add.sin_port = htons(port);
    
    // check for binding error 
    bind_status = bind(socket_des, (struct sockaddr *) &server_add, sizeof(server_add));
    if(bind_status == -1){
        perror("Server Binding Error: ");
        std::exit(EXIT_SUCCESS);   
    }
    
    //loop begins
    while(true)
    {
        std::cout<<"Server Waiting for the Client..."<<std::endl;
        listen_status = listen(socket_des, SOMAXCONN);

        if(listen_status == -1){
            perror("Server Listening Error ");
            std::exit(EXIT_SUCCESS);   
        }
        
        //cache->printCache();
        memset(&client_add, '\0', sizeof(client_add));
        clientLen = sizeof(client_add);
        accept_status = accept(socket_des, (struct sockaddr *) &client_add, &clientLen);

        if(accept_status == -1){
            perror("Server Accepting Error ");
            std::exit(EXIT_SUCCESS);   
        }

        char buffer[256];
        
        int n = read( accept_status,buffer,255);
        if (n < 0)
        {
            perror("ERROR reading from socket ");
            std::exit(EXIT_SUCCESS);
        }

        std::cout<<"Client " << inet_ntoa(client_add.sin_addr) << " is requesting file "<< buffer <<std::endl;  

        if (inCache(buffer, cache, accept_status))
        {
            std::cout<<"Cache hit. " << buffer << " sent to the client" << std::endl;
        }
        else if (inDirec(directory, buffer, accept_status, cache))
        {
            std::cout<<"Cache miss. " << buffer << " sent to the client" << std::endl;
        }
        else
        {
            std::cout<<"File " << buffer << " does not exist" << std::endl;
            char zero = '0';
            int errC = write(accept_status, &zero, 1);
            if (errC<0)
            {
                perror("Server Side Size writing error");
                std::exit(EXIT_SUCCESS);
            }
            
        }
            
    
    }
    
    close(socket_des);
    return 0;
}   


