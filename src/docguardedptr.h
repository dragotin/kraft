#ifndef DOCGUARDEDPTR
#define DOCGUARDEDPTR

#include <QPointer>

class KraftDoc;

// FIXME: Get rid of the DocGuardedPtr and use QScopedPointer in the many
// places where the pointer is deleted later.
typedef KraftDoc* DocGuardedPtr;

#endif

