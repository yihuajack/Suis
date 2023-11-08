#ifndef TMM_H
#define TMM_H

#include <QList>
#include <QString>
#include <limits>

#define EPSILON std::numeric_limits<double>::epsilon()

#define QWARNING_COMPLEX(c) \
qWarning() << #c << ": " << complex_to_string(c);

#endif // TMM_H
