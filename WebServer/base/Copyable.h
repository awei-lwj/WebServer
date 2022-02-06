#ifndef WEBSERVER_BASE_COPYABLE_H
#define WEBSERVER_BASE_COPYABLE_H

// A tag class emphasizes the objects are copyable.
// The empty base class optimization applies.
// Any derived class of copyable should be a value type.
class copyable
{

};

#endif // WEBSERVER_BASE_COPYABLE_H