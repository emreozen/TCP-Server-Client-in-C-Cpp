//header
#include "tcp_client.h"


//this function joins the directory and filename
std::string getFullDirect(char* direc, char *file)
{   
    std::string str1 = std::string(direc);
    std::string str2 = std::string(file); 
    str1.append("/");
    str1.append(str2);
    if(str1[0] != '.')
    {
        if (str1[0] !='/'){
            str1.insert(0,1,'/');            
        }
    
        str1.insert(0,1,'.');
    }    
    return str1;
}

//this function removes the first character of a char array
char *removeFirst (char *charNew) {
  char *str;
  
  if (strlen(charNew) == 0)
    str = charNew;
  else
    str = charNew + 1;
  return str;
}

//this function checks if the directory exists
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
 % tcp_client server_host server_port file_name directory
e.g.
% ./tcp_client 59.78.58.28 9089 homework1.html .
 */
int main(int argc, char**argv) {
    // Prints welcome message...
    std::cout << "Welcome To TCP Client..." << std::endl;

    /// Checks correct number of argument if not error message and quit
    if (argc != 5) { //4
        std::cout << std::endl << "4 Arguments expected:" << std::endl;
        for (int i = 1; i < argc; i++) {
            std::cout << i << ": " << argv[i] << std::endl;
        }
        std::cout <<"./tcp_client server_host server_port file_name directory"<<std::endl;
        
        std::exit(EXIT_SUCCESS);   
    }
    
    int socket_des, port, connect_des;
    char directory[512];
    
    //check directory size
    if (sizeof(argv[4])>512)
    {
        std::cout <<"Error: Directory should be less than 512 characters"<<std::endl;
        std::exit(EXIT_SUCCESS); 
    }
    
    strcpy(directory, argv[4]);
    

    //check port number
    port = atoi(argv[2]);
    if (port <=0 || port >=100000)
    {
        std::cout <<"Error: port no should be greater than zero and less than 100,000 "<<std::endl;
        std::exit(EXIT_SUCCESS); 
    }

    //check if directory exists
    dirExist(directory);
    
    char buffer[256];
    //check file name size
    if (sizeof(argv[3])>256)
    {
        std::cout <<"Error: file name should be less than 256 characters"<<std::endl;
        std::exit(EXIT_SUCCESS); 
    }
    
    strcpy(buffer, argv[3]);
    
    //file & directory opening
    FILE *outFile;
    std::string path = getFullDirect(directory, buffer);
    DIR *dir = NULL;
    struct dirent *dirCont = NULL;
    char *dirFormat = directory;
    
    if (directory[0] =='/')
        dirFormat = removeFirst(directory);
    
    dir = opendir(dirFormat);
    if (dir == NULL)
    {
        perror("Directory Does not exist ");
        std::exit(EXIT_SUCCESS);           
    }

    //create the socket
    socket_des = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_des == -1){
        perror("Client Socket Error ");
        std::exit(EXIT_SUCCESS);   
    }

    
    struct sockaddr_in destination;

    memset(&destination, '\0', sizeof(destination));
    destination.sin_addr.s_addr = inet_addr(argv[1]); 
    destination.sin_port = htons(port);
    destination.sin_family = AF_INET;
    
    //check if connection is ok
    connect_des = connect(socket_des, (struct sockaddr *) &destination, sizeof(destination));
    if(connect_des == -1){
        perror("Client Connection Error ");
        std::exit(EXIT_SUCCESS);   
    }
    

    
    //request the file
    int n = write( socket_des,buffer,sizeof(buffer));
    if (n < 0)
    {
        perror("ERROR SENDING filename ");
        exit(EXIT_SUCCESS);
    }


    size_t totalRec = 0;
    int lastRec = DATA_BLOCK -1;
    
    int errC;
    char fileSizeC[1024];
    
    errC = read(socket_des, fileSizeC, 1024);
    size_t fileSize = strtoul(fileSizeC, NULL,0);

    //check if the file exists on the server
    if (fileSize == 0)
    {
        std::string str = std::string("File ");
        str.append(buffer);
        str.append(" does not exist in the server");
        std::cout<<str<<std::endl;
        exit(EXIT_SUCCESS);
    }
    
    // check if there size transfer is ok
    if (errC<0)
    {
        perror("Client Side size reading error ");
        exit(1);
    }
    //check if file is opened
    outFile = fopen(path.c_str(), "wb");
    if (outFile == NULL)
    {
        perror("File Error ");
        exit(EXIT_SUCCESS);
    }
    //receiving the file 
    while (totalRec < fileSize)
    {
        char temp[DATA_BLOCK] = {0};
        memset(temp, 0, DATA_BLOCK);
        
        lastRec = recv(socket_des, temp, sizeof(temp),0);
        
        //check if data transfer is ok
        if (errC<0)
        {
            perror("Client Side reading error ");
            exit(EXIT_SUCCESS);
        }
        
        totalRec+=lastRec;
        
//        std::cout<<"RECEIVED" << lastRec<< " TOTAL " <<totalRec<<std::endl;

        //check if writing to file is successful
        size_t res = fwrite(temp, lastRec, 1, outFile);
        if (res != 1)
        {
            perror("Client Side writing to file error ");
            exit(EXIT_SUCCESS);            
        }

    }
    
    std::cout<<"File " << buffer << " saved" <<std::endl;
    
    //close the file and socket
    fclose(outFile);
    close(socket_des);
    return 0;
}
