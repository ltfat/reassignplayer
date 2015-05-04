/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef PLUGINBINARYDATA_H
#define PLUGINBINARYDATA_H

namespace PluginBinaryData
{
    extern const char*   default_lfb;
    const int            default_lfbSize = 71566;

    extern const char*   default_fgrad_lfb;
    const int            default_fgrad_lfbSize = 71566;

    extern const char*   default_tgrad_lfb;
    const int            default_tgrad_lfbSize = 71566;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 3;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();

    const int bufferLen = 2048;
}

#endif
