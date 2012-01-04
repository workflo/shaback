#ifndef SHABACK_H
#define SHABACK_H

#include "RuntimeConfig.h"
#include "Repository.h"

class Shaback
{
public:
    Shaback(RuntimeConfig& config);
    virtual ~Shaback();
    virtual void createRepository();
    static int deflate();
    static int inflate();

    Repository repository;
    
  protected:
    RuntimeConfig& config;
};

#endif // SHABACK_H
