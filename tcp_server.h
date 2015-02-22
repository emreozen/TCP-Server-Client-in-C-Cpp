
#ifndef TCP_SERVER_H
#define	TCP_SERVER_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define CACHE_SIZE 67108864
#define DATA_BLOCK 1024

// And object for each file in the Cache
class CacheObj {
    public:
        std::string fileName;
        void * filePtr;
        size_t size;
        
    
       CacheObj(std::string fName, void *ptr, size_t s )
       {
           fileName = fName;
           filePtr = ptr;
           size = s;
       
       }
       ~CacheObj()
       {
           free(filePtr);
       }
};

// And object to contain the whole cache

class Cache{
    public:
        size_t totalSize;
        std::map<std::string, CacheObj*> list;
    
        Cache()
        {
            totalSize = 0;
            
        }
        ~Cache()
        {
            std::map<std::string, CacheObj*>::iterator it;
            for(it = list.begin(); it != list.end(); it++)
            {
                it->second->~CacheObj();
            }
        }
        void printCache()
        {
            std::cout<< "Printing the cache " <<std::endl;
            std::map<std::string, CacheObj*>::iterator it;
            for(it = list.begin(); it != list.end(); it++)
            {
                std::cout<< it->first <<std::endl;
            }
                std::cout<< "Total size "<< totalSize <<std::endl;
        }
        bool inCache(char* file)
        {
            std::string temp(file);

            if (list.find(temp) != list.end())
                return true;
            return false;    
        }
        void arrangeSize(size_t incomingSize)
        {
            if (CACHE_SIZE >= (incomingSize + totalSize))
            {
                return;
            }
            else
            {
                std::map<std::string, CacheObj*>::iterator it;
                for(it = list.begin(); it != list.end(); it++)
                {
                    totalSize -= it->second->size;
//                    std::cout<<"erasing  " <<it->first <<" new size"<< totalSize <<std::endl;
                    it->second->~CacheObj();
                    list.erase(it);
                    
                    if (CACHE_SIZE >= (totalSize + incomingSize))
                        break;
                }
            }
        }
        void add(std::string fname, size_t fsize, void *fptr)
        {
            
            list[fname] = new CacheObj(fname,fptr,fsize);
            
            totalSize += fsize;
        }
    
};

#endif	/* TCP_SERVER_H */

