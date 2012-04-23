#ifndef SocketPACKAGEELEMENT_HH
#define SocketPACKAGEELEMENT_HH
#include <click/element.hh>

CLICK_DECLS

class PTPd2PackageElement : public Element { public:

    PTPd2PackageElement();		
    ~PTPd2PackageElement();		

    const char *class_name() const	{ return "PTPd2PackageElement"; }

    int initialize(ErrorHandler *errh);

};


CLICK_ENDDECLS
#endif
