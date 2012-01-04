#ifndef SHABACK_Properties_H
#define SHABACK_Properties_H

#include <string>
#include <map>
#include "InputStream.h"
#include "File.h"

class Properties
{
  public:
    Properties();
    void load(InputStream& in);
    void load(File& f);
    std::string getProperty(std::string& key);
    std::string getProperty(const char* key);

    virtual ~Properties();

  private:
    std::map<std::string, std::string> map;
};
#endif // SHABACK_Properties_H
