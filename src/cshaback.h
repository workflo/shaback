#ifndef cshaback_H
#define cshaback_H

#include "RuntimeConfig.h"
#include "Repository.h"

class Shaback
{
public:
    Shaback(RuntimeConfig& config);
    virtual ~Shaback();
    virtual void createRepository();
    
    Repository repository;
    
  protected:
    RuntimeConfig& config;
};

#endif // cshaback_H
