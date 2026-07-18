#ifndef ALSDISTANT_H
#define ALSDISTANT_H

#include <QFile>

class AlsDistant
{
public:
    AlsDistant();
    int realDistantValue();

private:
    int readAlsDistantValue();
};

#endif // ALSDISTANT_H
